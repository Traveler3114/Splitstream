#include "DefaultPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Actors/LobbyPlatformActor.h"
#include "Components/WidgetComponent.h"
#include "Widgets/Lobby/PlayerLobbyInfo.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"

ADefaultPlayerState::ADefaultPlayerState()
{
    AbilitySystemComponent = CreateDefaultSubobject<UDefaultAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AttributeSet = CreateDefaultSubobject<UDefaultAttributeSet>(TEXT("AttributeSet"));

    NetUpdateFrequency = 30.f;
    MinNetUpdateFrequency = 30.f;
}

UAbilitySystemComponent* ADefaultPlayerState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
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
            if (!HasAuthority())
            {
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
}

void ADefaultPlayerState::RefreshLobbyInfoUI()
{
    ApplyLobbyInfoToWidget();

    if (AssignedPlatform)
    {
        AssignedPlatform->ForceNetUpdate();
    }
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

void ADefaultPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION_NOTIFY(ADefaultPlayerState, AssignedPlatform, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME(ADefaultPlayerState, bIsReady);
    DOREPLIFETIME(ADefaultPlayerState, TeamTag);
}