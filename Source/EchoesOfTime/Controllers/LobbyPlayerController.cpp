#include "LobbyPlayerController.h"
#include "Widgets/Lobby/LobbyUI.h"
#include "Actors/LobbyPlatformActor.h"
#include "GameModes/LobbyGameMode.h"
#include "Widgets/Lobby/PlayerLobbyInfo.h"
#include "Components/WidgetComponent.h"
#include "DefaultPlayerState.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

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
            LobbyUIInstance = LobbyUI; // store for later use

            // Host sees Start button; clients don�t
            if (HasAuthority())
            {
                LobbyUI->SetStartButtonVisibility(ESlateVisibility::Visible);
                // Ensure disabled by default; GameMode will enable when everyone is ready
                LobbyUI->SetStartButtonEnabled(false);
            }
            else
            {
                LobbyUI->SetStartButtonVisibility(ESlateVisibility::Collapsed);
            }
        }
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
