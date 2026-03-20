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