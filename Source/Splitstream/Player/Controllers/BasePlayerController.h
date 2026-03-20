#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BasePlayerController.generated.h"

/**
 * Base player controller providing shared functionality for all game modes.
 * Handles main menu navigation and loading screen display logic
 * shared between ADefaultPlayerController and ALobbyPlayerController.
 */
UCLASS()
class SPLITSTREAM_API ABasePlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable)
    virtual void RequestLeaveToMainMenu();

 //   UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
 //   TSubclassOf<class UUserWidget> LoadingWidgetClass;

 //   UPROPERTY()
 //   UUserWidget* LoadingWidgetInstance = nullptr;

 //   UFUNCTION(Client,Reliable)
 //   void ClientShowLoadingWidget();

 //   UFUNCTION(BlueprintCallable)
	//void ShowLoadingWidget();
};