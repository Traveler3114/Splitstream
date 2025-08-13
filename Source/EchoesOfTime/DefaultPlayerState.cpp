#include "DefaultPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Actors/LobbyPlatformActor.h"
#include "Components/WidgetComponent.h"
#include "Widgets/Lobby/PlayerLobbyInfo.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"

// Static cached gameplay tags for teams
namespace CachedGameplayTags
{
	static FGameplayTag TeamFuture;
	static FGameplayTag TeamPast;
	static bool bTagsInitialized = false;

	void InitializeTags()
	{
		if (!bTagsInitialized)
		{
			TeamFuture = FGameplayTag::RequestGameplayTag(FName("Team.Future"));
			TeamPast = FGameplayTag::RequestGameplayTag(FName("Team.Past"));
			bTagsInitialized = true;
		}
	}
}

ADefaultPlayerState::ADefaultPlayerState()
{
    AbilitySystemComponent = CreateDefaultSubobject<UDefaultAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AttributeSet = CreateDefaultSubobject<UDefaultAttributeSet>(TEXT("AttributeSet"));

	// Initialize cached gameplay tags
	CachedGameplayTags::InitializeTags();

	// Lower replication frequencies for lobby (mostly static state)
	SetNetUpdateFrequency(10.f);
	SetMinNetUpdateFrequency(5.f);
}

UAbilitySystemComponent* ADefaultPlayerState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void ADefaultPlayerState::ServerSetReadyState_Implementation(bool bReady)
{
    bIsReady = bReady;

    // Update server immediately
    ApplyLobbyInfoToWidget();
    if (HasAuthority())
    {
        OnPlayerReady.Broadcast();
    }

    // Normal replication will handle updates (no need for immediate flush)
}

void ADefaultPlayerState::ServerSetTeamTag_Implementation(FGameplayTag NewTeamTag)
{
    TeamTag = NewTeamTag;

    // Update ASC tags (authoritative on server) using cached tags
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->RemoveLooseGameplayTag(CachedGameplayTags::TeamFuture);
        AbilitySystemComponent->RemoveLooseGameplayTag(CachedGameplayTags::TeamPast);
        AbilitySystemComponent->AddLooseGameplayTag(NewTeamTag);
    }

    // Update server UI immediately
    ApplyLobbyInfoToWidget();

    // Normal replication will handle updates
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
    // Use existing ownership instead of O(n) iteration
    return Cast<AController>(GetOwner());
}

void ADefaultPlayerState::RefreshLobbyInfoUI()
{
    ApplyLobbyInfoToWidget();
    // Normal replication will handle updates
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
}