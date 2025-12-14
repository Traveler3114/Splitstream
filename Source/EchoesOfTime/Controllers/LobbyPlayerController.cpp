#include "LobbyPlayerController.h"
#include "Widgets/Lobby/LobbyUI.h"
#include "Blueprint/UserWidget.h"
#include "GameModes/LobbyGameMode.h"
#include "GameFramework/PlayerState.h"
#include "Engine/Engine.h"

void ALobbyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalController()) return;

    if (LobbyUIClass)
    {
        LobbyUIInstance = CreateWidget<ULobbyUI>(this, LobbyUIClass);
        if (LobbyUIInstance)
        {
            LobbyUIInstance->AddToViewport();

            if (HasAuthority())
            {
                LobbyUIInstance->SetStartButtonVisibility(ESlateVisibility::Visible);
                LobbyUIInstance->SetStartButtonEnabled(false);
            }
            else
            {
                LobbyUIInstance->SetStartButtonVisibility(ESlateVisibility::Collapsed);
            }
        }
    }
}

void ALobbyPlayerController::ClientSetStartButtonEnabled_Implementation(bool bEnabled)
{
    if (LobbyUIInstance)
    {
        LobbyUIInstance->SetStartButtonEnabled(bEnabled);
    }
}

void ALobbyPlayerController::ServerStartGame_Implementation()
{
    if (HasAuthority())
    {
        if (ALobbyGameMode* GM = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
        {
            GM->StartGameIfAllowed(this);
        }
    }
}

void ALobbyPlayerController::ClientShowLoadingScreen_Implementation()
{
    if (LoadingWidgetClass)
    {
        if (UUserWidget* LoadingWidget = CreateWidget<UUserWidget>(this, LoadingWidgetClass, TEXT("LoadingWidget")))
        {
            LoadingWidget->AddToViewport();
        }
    }
}

void ALobbyPlayerController::ServerLeaveLobby_Implementation()
{
    if (!HasAuthority()) return;

    // If this is the listen server host, leave for everyone (GameMode handles showing loading + travel)
    if (IsLocalController())
    {
        if (ALobbyGameMode* GM = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
        {
            GM->HostLeaveLobby();
        }
    }
    else
    {
        // Remote client leaving: show loading screen, then travel this client to main menu
        FString MenuPath = TEXT("/Game/Maps/MainMenuMap");
        if (const ALobbyGameMode* GM = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
        {
            MenuPath = GM->MainMenuMapPath;
        }

        ClientShowLoadingScreen();
        ClientTravel(MenuPath, TRAVEL_Absolute);
        // Server will receive disconnect and process Logout() cleanup
    }
}

void ALobbyPlayerController::ServerKickPlayer_Implementation(APlayerState* TargetPS)
{
    if (!HasAuthority() || !TargetPS) return;

    // Only allow the listen-server host to kick
    if (!IsLocalController())
    {
        return;
    }

    if (ALobbyGameMode* GM = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
    {
        GM->KickPlayerByPlayerState(TargetPS);
    }
}