#include "LobbyPlayerController.h"
#include "Widgets/Lobby/LobbyUI.h"
#include "Actors/LobbyPlatformActor.h"
#include "GameModes/LobbyGameMode.h"
#include "Widgets/Lobby/PlayerLobbyInfo.h"
#include "Components/WidgetComponent.h"
#include "DefaultPlayerState.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

void ALobbyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalController())
    {
        return;
    }

    if (LobbyUIClass)
    {
        ULobbyUI* LobbyUI = CreateWidget<ULobbyUI>(this, LobbyUIClass);
        if (LobbyUI)
        {
            LobbyUI->AddToViewport();
            if (HasAuthority())
            {
                LobbyUI->SetStartButtonVisibility(ESlateVisibility::Visible);
            }
        }
    }
}

void ALobbyPlayerController::ServerSetReadyState_Implementation(bool bReady)
{
    bIsReady = bReady;
    if (AssignedPlatform)
    {
        AssignedPlatform->SetPlayerReadyState(bIsReady);
    }
    OnRep_ReadyState();
    if (HasAuthority())
    {
        OnPlayerReady.Broadcast();
    }
}

void ALobbyPlayerController::OnRep_ReadyState()
{
    if (AssignedPlatform)
    {
        AssignedPlatform->SetPlayerReadyState(bIsReady);
    }
}

void ALobbyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALobbyPlayerController, bIsReady);
    DOREPLIFETIME(ALobbyPlayerController, AssignedPlatform);
    DOREPLIFETIME(ALobbyPlayerController, TeamTag); // Add this line
}


void ALobbyPlayerController::ServerSetTeamTag_Implementation(FGameplayTag NewTeamTag)
{
    TeamTag = NewTeamTag;
    OnRep_TeamTag();

    // Set the tag on the assigned platform so it replicates to all clients
    if (AssignedPlatform)
    {
        AssignedPlatform->TeamTag = NewTeamTag;
        AssignedPlatform->OnRep_PlayerInfo(); // Update UI immediately on server
    }

    // Add the tag to the PlayerState's ASC
    if (ADefaultPlayerState* PS = Cast<ADefaultPlayerState>(PlayerState))
    {
        if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
        {
            // Remove old team tags (if you only allow one team tag at a time)
            ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Team.Future")));
            ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Team.Past")));
            // Add the new team tag
            ASC->AddLooseGameplayTag(NewTeamTag);
        }
    }
}

void ALobbyPlayerController::OnRep_TeamTag()
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