// DefaultPlayerController.h
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
    virtual void OnRep_PlayerState() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    void TogglePauseMenu();
    void BindPauseMenuEsc();
    void UnbindPauseMenuEsc();

    void BindAttributeDelegates();
    void BindGameplayTagDelegates();

    void OnHealthChanged(const struct FOnAttributeChangeData& Data);
    void OnIllegalTagChanged(const struct FGameplayTag Tag, int32 NewCount);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UPauseMenuWidget> PauseMenuWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UCalendarWidget> CalendarWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UUserWidget> LoadingWidgetClass;

    UPROPERTY()
    UUserWidget* LoadingWidgetInstance = nullptr;

    UPROPERTY(BlueprintReadWrite)
    UCalendarWidget* CalendarWidgetInstance;

    bool bIsPauseMenuOpen = false;

    UFUNCTION(Client, Reliable)
    void ClientShowCalendarWidget(const TArray<FCalendarCivilianRecord>& CivilianDateRecords);

    UFUNCTION(Client, Reliable)
    void ClientUpdateDetectionWidgetForGuard(AActor* Guard, float Progress, bool bIsLocked = false, float AngleDegrees = 0.0f);

    UFUNCTION(Server, Reliable)
    void ServerTryLockPick(AActor* TargetDoor, float Angle);

    UFUNCTION(Client, Reliable)
    void ClientShowLoadingScreen();

    // --- Pre-alarm client RPCs ---
    // Renamed parameter to avoid shadowing the member variable
    UFUNCTION(Client, Reliable)
    void ClientStartPreAlarm(float InPreAlarmEndTime);

    UFUNCTION(Client, Reliable)
    void ClientCancelPreAlarm();

private:
    UPROPERTY()
    class ACharacterHUD* CharacterHUD;

    UPROPERTY()
    UPauseMenuWidget* PauseMenuWidget;

    FDelegateHandle IllegalTagDelegateHandle;

    UFUNCTION()
    void HandlePauseMenuResumed();

    FTimerHandle PingUpdateTimerHandle;
    void UpdatePingOnOverlay();

    // Alarm UI handling (final alarm before restart)
    UPROPERTY()
    float AlarmEndTime = 0.f;

    FTimerHandle AlarmUpdateTimerHandle;

    UFUNCTION()
    void HandleAlarmStarted(float InAlarmEndTime);

    UFUNCTION()
    void HandleAlarmCanceled(); // handles alarm cancel from server

    void UpdateAlarmUI();

    // Pre-alarm UI handling (guard detection -> pre-alarm)
    UPROPERTY()
    float PreAlarmEndTime = 0.f;

    FTimerHandle PreAlarmUpdateTimerHandle;

    UFUNCTION()
    void HandlePreAlarmStarted(float InPreAlarmEndTime);

    UFUNCTION()
    void HandlePreAlarmCanceled();

    void UpdatePreAlarmUI();
};