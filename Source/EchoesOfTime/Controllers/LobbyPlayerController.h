#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "LobbyPlayerController.generated.h"

class ULobbyUI;

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
    void OnStartGame();

    UFUNCTION(Server, Reliable)
    void ServerRequestLeaveLobby();

    // NEW: Server RPC for validated kick functionality
    UFUNCTION(Server, Reliable)
    void ServerKickPlayer(const FString& TargetPlayerId);

    // NEW: client RPC to toggle Start button
    UFUNCTION(Client, Reliable)
    void ClientSetStartButtonEnabled(bool bEnabled);

private:
    // NEW: keep a reference to the created Lobby UI on the local client
    UPROPERTY()
    ULobbyUI* LobbyUIInstance = nullptr;
};