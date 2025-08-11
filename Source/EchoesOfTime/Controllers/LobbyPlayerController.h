#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "LobbyPlayerController.generated.h"


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


	UFUNCTION(Client,Reliable)
	void OnStartGame();

    // Add this public method declaration:
    UFUNCTION(Server, Reliable)
    void ServerRequestLeaveLobby();
};