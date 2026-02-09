#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Controllers/BasePlayerController.h"
#include "LobbyPlayerController.generated.h"

class ULobbyUI;
class APlayerState;

UCLASS()
class SPLITSTREAM_API ALobbyPlayerController : public ABasePlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class ULobbyUI> LobbyUIClass;

    UFUNCTION(Client, Reliable)
    void ClientSetStartButtonEnabled(bool bEnabled);

    UFUNCTION(Server, Reliable)
    void ServerStartGame();

    // Host-only: kick a player by PlayerState (called from PlayerLobbyInfo)
    UFUNCTION(Server, Reliable)
    void ServerKickPlayer(APlayerState* TargetPS);

private:
    UPROPERTY()
    ULobbyUI* LobbyUIInstance = nullptr;

};