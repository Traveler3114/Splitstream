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
	void BindAttributeDelegates();

    void OnHealthChanged(const struct FOnAttributeChangeData& Data);

	virtual void OnRep_PlayerState() override;

    void BindPauseMenuEsc();
    void UnbindPauseMenuEsc();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UPauseMenuWidget> PauseMenuWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UCalendarWidget> CalendarWidgetClass;

    UPROPERTY(BlueprintReadWrite)
    UCalendarWidget* CalendarWidgetInstance;

    bool bIsPauseMenuOpen = false;

    UFUNCTION(Client, Reliable)
    void ClientShowCalendarWidget(const TArray<FCalendarCivilianRecord>& CivilianDateRecords);

    UFUNCTION(Client, Reliable)
    void ClientUpdateDetectionWidgetForGuard(AActor* Guard, float Progress, bool bIsLocked = false, float AngleDegrees = 0.0f);

    UFUNCTION(Server, Reliable)
    void ServerTryLockPick(AActor* TargetDoor, float Angle);
private:
    UPROPERTY()
    class ACharacterHUD* CharacterHUD;

    UPROPERTY()
    UPauseMenuWidget* PauseMenuWidget;

    UFUNCTION()
    void HandlePauseMenuResumed();
};