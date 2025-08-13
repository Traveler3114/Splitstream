#include "DefaultPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Actors/LobbyPlatformActor.h"
#include "Components/WidgetComponent.h"
#include "Widgets/Lobby/PlayerLobbyInfo.h"
#include "GameStates/LobbyGameState.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"

ADefaultPlayerState::ADefaultPlayerState()
{
    AbilitySystemComponent = CreateDefaultSubobject<UDefaultAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AttributeSet = CreateDefaultSubobject<UDefaultAttributeSet>(TEXT("AttributeSet"));

	SetNetUpdateFrequency(30.f);
	SetMinNetUpdateFrequency(30.f);
}

UAbilitySystemComponent* ADefaultPlayerState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void ADefaultPlayerState::ServerSetReadyState_Implementation(bool bReady)
{
    // Rate limiting check
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastReadyToggleTime < 1.0f) // 1 second rate limit
    {
        UE_LOG(LogLobby, Warning, TEXT("Ready toggle rate limited for player %s"), *GetPlayerName());
        return;
    }
    LastReadyToggleTime = CurrentTime;

    bIsReady = bReady;

    // Update GameState roster
    if (ALobbyGameState* LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>())
    {
        const FString PlayerId = GetUniqueId().ToString();
        LobbyGameState->UpdatePlayerRosterEntry(PlayerId, TeamTag, bIsReady);
    }

    // Legacy: Update server immediately and broadcast for backward compatibility
    ApplyLobbyInfoToWidget();
    if (HasAuthority())
    {
        OnPlayerReady.Broadcast();
    }

    // Remove ForceNetUpdate calls - rely on RepNotifies and FastArray replication
}

void ADefaultPlayerState::ServerSetTeamTag_Implementation(FGameplayTag NewTeamTag)
{
    // Rate limiting check
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastTeamChangeTime < 2.0f) // 2 second rate limit for team changes
    {
        UE_LOG(LogLobby, Warning, TEXT("Team change rate limited for player %s"), *GetPlayerName());
        return;
    }
    LastTeamChangeTime = CurrentTime;

    // Check if team changes are allowed based on lobby phase
    if (ALobbyGameState* LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>())
    {
        if (!LobbyGameState->CanPlayersChangeTeams())
        {
            UE_LOG(LogLobby, Warning, TEXT("Team changes not allowed in current lobby phase"));
            return;
        }
    }

    TeamTag = NewTeamTag;

    // Update GameState roster
    if (ALobbyGameState* LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>())
    {
        const FString PlayerId = GetUniqueId().ToString();
        LobbyGameState->UpdatePlayerRosterEntry(PlayerId, TeamTag, bIsReady);
    }

    // Update ASC tags (authoritative on server)
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Team.Future")));
        AbilitySystemComponent->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Team.Past")));
        AbilitySystemComponent->AddLooseGameplayTag(NewTeamTag);
    }

    // Legacy: Update server UI immediately for backward compatibility
    ApplyLobbyInfoToWidget();

    // Remove ForceNetUpdate calls - rely on RepNotifies and FastArray replication
}

void ADefaultPlayerState::ApplyLobbyInfoToWidget()
{
    if (!AssignedPlatform || !AssignedPlatform->PlayerInfoWidget)
    {
        return;
    }

    AssignedPlatform->PlayerInfoWidget->InitWidget();

    if (UUserWidget* UserWidget = AssignedPlatform->PlayerInfoWidget->GetUserWidgetObject())
    {
        if (UPlayerLobbyInfo* LobbyInfo = Cast<UPlayerLobbyInfo>(UserWidget))
        {
            // Basic fields
            LobbyInfo->SetPlayerName(FText::FromString(GetPlayerName()));
            LobbyInfo->SetReadyState(bIsReady);
            LobbyInfo->SetTeamTag(TeamTag);

            // Kick button: visible on server, hidden on clients
            LobbyInfo->SetKickButtonVisible(HasAuthority());

            // Avatar: fetch on clients via your BP event; cache to avoid repeated calls
            if (!CachedAvatarTexture)
            {
                AController* OwningController = FindOwningController();
                CachedAvatarTexture = GetPlayerAvatar(OwningController);
            }
            if (CachedAvatarTexture)
            {
                LobbyInfo->SetAvatarTexture(CachedAvatarTexture);
            }
        }
    }
}
AController* ADefaultPlayerState::FindOwningController() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC && PC->PlayerState == this)
        {
            return PC;
        }
    }
    return nullptr;
}

void ADefaultPlayerState::RefreshLobbyInfoUI()
{
    ApplyLobbyInfoToWidget();
    // Remove ForceNetUpdate - rely on RepNotifies
}

void ADefaultPlayerState::OnRep_ReadyState()
{
    ApplyLobbyInfoToWidget();
}

void ADefaultPlayerState::OnRep_TeamTag()
{
    ApplyLobbyInfoToWidget();
}

void ADefaultPlayerState::OnRep_PlayerName()
{
    Super::OnRep_PlayerName();
    ApplyLobbyInfoToWidget();
}

void ADefaultPlayerState::OnRep_AssignedPlatform()
{
    ApplyLobbyInfoToWidget();
}


UTexture2D* ADefaultPlayerState::GetPlayerAvatar_Implementation(AController* NewController)
{
    // This is a Blueprint-implementable event, so you can override it in your BP
    return nullptr; // Default implementation returns null
}


void ADefaultPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION_NOTIFY(ADefaultPlayerState, AssignedPlatform, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME(ADefaultPlayerState, bIsReady);
    DOREPLIFETIME(ADefaultPlayerState, TeamTag);
    DOREPLIFETIME(ADefaultPlayerState, CachedAvatarTexture);
    DOREPLIFETIME(ADefaultPlayerState, AvatarId);
    DOREPLIFETIME(ADefaultPlayerState, RoleTag);
}