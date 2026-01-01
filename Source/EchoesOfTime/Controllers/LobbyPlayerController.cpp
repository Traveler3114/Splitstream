#include "LobbyPlayerController.h"
#include "Widgets/Lobby/LobbyUI.h"
#include "Blueprint/UserWidget.h"
#include "GameModes/LobbyGameMode.h"
#include "GameStates/LobbyGameState.h"
#include "GameFramework/PlayerState.h"
#include "AdvancedSessionsLibrary.h"
#include "OnlineSessionSettings.h"
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
            IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
            if (!Subsystem) return;

            IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
            if (!Session.IsValid()) return;

            FNamedOnlineSession* SessionData = Session->GetNamedSession(NAME_GameSession);
            if (!SessionData) return;

            FString MapName;
            if (SessionData->SessionSettings.Get(FName("LEVELNAME"), MapName))
            {
                if (LobbyUIInstance)
                {
                    LobbyUIInstance->SetMapName(FText::FromString(MapName));
                }
            }
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

void ALobbyPlayerController::RequestLeaveToMainMenu()
{
    if (HasAuthority() && IsLocalController())
    {
        // Host: tell GameMode
        if (ALobbyGameMode* GM = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
        {
            GM->HostLeaveLobby();
        }
    }
    else
    {
        // CLIENT: only disconnect self, do NOT trigger session/network destroy
        if (ALobbyGameState* GS = GetWorld()->GetGameState<ALobbyGameState>())
        {
            ClientShowLoadingScreen();
            ClientTravel(GS->MainMenuMapPath, TRAVEL_Absolute);
        }
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