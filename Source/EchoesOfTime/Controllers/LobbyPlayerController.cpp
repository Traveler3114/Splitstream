#include "LobbyPlayerController.h"
#include "Widgets/Lobby/LobbyUI.h"
#include "Actors/LobbyPlatformActor.h"
#include "GameModes/LobbyGameMode.h"
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
        if (ALobbyGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ALobbyGameMode>() : nullptr)
        {
            GM->CheckAllPlayersReady();
        }
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
}
