#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DefaultPlayerController.generated.h"

class UPauseMenuWidget;

UCLASS()
class ECHOESOFTIME_API ADefaultPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    ADefaultPlayerController();

    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    void TogglePauseMenu();

    void BindPauseMenuEsc();
    void UnbindPauseMenuEsc();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UPauseMenuWidget> PauseMenuWidgetClass;

    bool bIsPauseMenuOpen = false;

    UFUNCTION(Server, Reliable)
    void ServerLockPickConfirm(AActor* DoorActor, float Angle);

private:
    UPROPERTY()
    class ACharacterHUD* CharacterHUD;

    UPROPERTY()
    UPauseMenuWidget* PauseMenuWidget;

    UFUNCTION()
    void HandlePauseMenuResumed();
};