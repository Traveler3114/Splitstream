#include "DefaultPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Actors/LobbyPlatformActor.h"
#include "Components/WidgetComponent.h"
#include "Widgets/Lobby/PlayerLobbyInfo.h"

ADefaultPlayerState::ADefaultPlayerState()
{
    AbilitySystemComponent = CreateDefaultSubobject<UDefaultAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AttributeSet = CreateDefaultSubobject<UDefaultAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* ADefaultPlayerState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}


void ADefaultPlayerState::ServerSetReadyState_Implementation(bool bReady)
{
    bIsReady = bReady;
    OnRep_ReadyState();
    if (HasAuthority())
    {
        OnPlayerReady.Broadcast();
    }
}

void ADefaultPlayerState::OnRep_ReadyState()
{
    if (AssignedPlatform && AssignedPlatform->PlayerInfoWidget)
    {
        UUserWidget* UserWidget = AssignedPlatform->PlayerInfoWidget->GetUserWidgetObject();
        if (UPlayerLobbyInfo* LobbyInfo = Cast<UPlayerLobbyInfo>(UserWidget))
        {
			LobbyInfo->SetReadyState(bIsReady); // Assuming you have a method to set the ready state in your widget
        }
    }
}


void ADefaultPlayerState::ServerSetTeamTag_Implementation(FGameplayTag NewTeamTag)
{
    TeamTag = NewTeamTag;
    OnRep_TeamTag();

    // Set the tag on the assigned platform so it replicates to all clients
    if (AssignedPlatform)
    {
        //AssignedPlatform->TeamTag = NewTeamTag;
        //AssignedPlatform->OnRep_PlayerInfo(); // Update UI immediately on server
    }

    if (AbilitySystemComponent)
    {
        // Remove old team tags (if you only allow one team tag at a time)
        AbilitySystemComponent->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Team.Future")));
        AbilitySystemComponent->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Team.Past")));
        // Add the new team tag
        AbilitySystemComponent->AddLooseGameplayTag(NewTeamTag);
    }
}

void ADefaultPlayerState::OnRep_TeamTag()
{
    // Update your UI here, e.g.:
    if (AssignedPlatform && AssignedPlatform->PlayerInfoWidget)
    {
        UUserWidget* UserWidget = AssignedPlatform->PlayerInfoWidget->GetUserWidgetObject();
        if (UPlayerLobbyInfo* LobbyInfo = Cast<UPlayerLobbyInfo>(UserWidget))
        {
            LobbyInfo->SetTeamTag(TeamTag); // You need to implement SetTeamTag in your widget
        }
    }
}

void ADefaultPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION_NOTIFY(ADefaultPlayerState, AssignedPlatform, COND_None, REPNOTIFY_Always);
    //DOREPLIFETIME(ADefaultPlayerState, AssignedPlatform);
    DOREPLIFETIME(ADefaultPlayerState, bIsReady);
    DOREPLIFETIME(ADefaultPlayerState, TeamTag);
}

void ADefaultPlayerState::OnRep_AssignedPlatform()
{
    // When AssignedPlatform is set, update the widget with current values
    OnRep_ReadyState();
    OnRep_TeamTag();
}