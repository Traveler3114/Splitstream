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

            // Host sees Start button; clients don’t
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

void ALobbyPlayerController::ServerRequestLeaveLobby_Implementation()
{
    ADefaultPlayerState* PS = Cast<ADefaultPlayerState>(PlayerState);
    if (PS && PS->AssignedPlatform)
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

// NEW: Client RPC called by server to toggle Start button on host client
void ALobbyPlayerController::ClientSetStartButtonEnabled_Implementation(bool bEnabled)
{
    if (LobbyUIInstance)
    {
        LobbyUIInstance->SetStartButtonEnabled(bEnabled);
        return;
    }

    // Fallback if UI pointer wasn’t cached
    TArray<UUserWidget*> FoundWidgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, ULobbyUI::StaticClass(), false);
    for (UUserWidget* Widget : FoundWidgets)
    {
        if (ULobbyUI* LobbyUI = Cast<ULobbyUI>(Widget))
        {
            LobbyUI->SetStartButtonEnabled(bEnabled);
        }
    }
}