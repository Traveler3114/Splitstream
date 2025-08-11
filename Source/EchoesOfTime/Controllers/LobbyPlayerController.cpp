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

void ALobbyPlayerController::ServerRequestLeaveLobby_Implementation()
{
    ADefaultPlayerState* PS = Cast<ADefaultPlayerState>(PlayerState);
    if (PS->AssignedPlatform)
    {
        if (PS->AssignedPlatform->OccupyingPawn)
        {
            PS->AssignedPlatform->OccupyingPawn->Destroy();
            PS->AssignedPlatform->OccupyingPawn = nullptr;
        }
        if (PS->AssignedPlatform->PlayerInfoWidget)
        {
            PS->AssignedPlatform->PlayerInfoWidget->SetVisibility(false);
        }
        PS->AssignedPlatform = nullptr;
    }
}






void ALobbyPlayerController::OnStartGame_Implementation()
{
    if (LoadingWidgetClass)
    {
        UUserWidget* LoadingWidget = CreateWidget<UUserWidget>(this, LoadingWidgetClass, TEXT("LoadingWidget"));
        if (LoadingWidget)
        {
            LoadingWidget->AddToViewport();
        }
    }
}