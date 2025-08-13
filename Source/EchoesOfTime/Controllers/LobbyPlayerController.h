#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
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
	void ClientSetStartButtonEnabled(bool bEnabled);

	UFUNCTION(Server, Reliable)
	void ServerStartGame();

	// Show loading screen before travel
	UFUNCTION(Client, Reliable)
	void ClientShowLoadingScreen();

private:
	UPROPERTY()
	ULobbyUI* LobbyUIInstance = nullptr;
};