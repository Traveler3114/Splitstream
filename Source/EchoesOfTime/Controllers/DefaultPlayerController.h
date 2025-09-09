#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DefaultPlayerController.generated.h"

class UPauseMenuWidget;
class UCalendarWidget; // <-- Add this line

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

    // --- Add these for calendar widget support ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UCalendarWidget> CalendarWidgetClass;

    UPROPERTY(BlueprintReadWrite)
    UCalendarWidget* CalendarWidgetInstance;

    // ---------------------------------------------

    bool bIsPauseMenuOpen = false;

    UFUNCTION(Server, Reliable)
    void ServerLockPickConfirm(AActor* DoorActor, float Angle);

    UFUNCTION(Client, Reliable)
    void ClientShowCalendarWidget(int32 Year, int32 Month, int32 Day, const FString& StaffName);

private:
    UPROPERTY()
    class ACharacterHUD* CharacterHUD;

    UPROPERTY()
    UPauseMenuWidget* PauseMenuWidget;

    UFUNCTION()
    void HandlePauseMenuResumed();
};