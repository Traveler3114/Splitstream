#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

class ULobbyUI;
class APlayerState;

UCLASS()
class ECHOESOFTIME_API ALobbyPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class ULobbyUI> LobbyUIClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UUserWidget> LoadingWidgetClass;

    UFUNCTION(Client, Reliable)
    void ClientSetStartButtonEnabled(bool bEnabled);

    UFUNCTION(Server, Reliable)
    void ServerStartGame();

    // Show loading screen locally on client
    UFUNCTION(Client, Reliable)
    void ClientShowLoadingScreen();

    // Called by UI when user presses Leave
    UFUNCTION(Server, Reliable)
    void ServerLeaveLobby();

    // Host-only: kick a player by PlayerState (called from PlayerLobbyInfo)
    UFUNCTION(Server, Reliable)
    void ServerKickPlayer(APlayerState* TargetPS);

private:
    UPROPERTY()
    ULobbyUI* LobbyUIInstance = nullptr;
};