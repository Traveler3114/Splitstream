#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BasePlayerController.generated.h"

UCLASS()
class SPLITSTREAM_API ABasePlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable)
    virtual void RequestLeaveToMainMenu();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UUserWidget> LoadingWidgetClass;

    UPROPERTY()
    UUserWidget* LoadingWidgetInstance = nullptr;

    UFUNCTION(Client,Reliable)
    void ClientShowLoadingWidget();

    UFUNCTION(BlueprintCallable)
	void ShowLoadingWidget();
};