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

    // ============================================
    // Unreal Engine Overrides
    // ============================================
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void OnRep_PlayerState() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // ============================================
    // UI Widget Classes
    // ============================================
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

    // ============================================
    // UI Management
    // ============================================
    bool bIsPauseMenuOpen = false;

    void TogglePauseMenu();

    UFUNCTION(Client, Reliable)
    void ClientShowCalendarWidget(const TArray<FCalendarDateRecord>& CivilianDateRecords);

    UFUNCTION(Client, Reliable, BlueprintCallable)
    void ClientUpdateDetectionWidget(AActor* DetectorActor, float Progress, bool bIsLocked = false, float AngleDegrees = 0.0f);

    UFUNCTION(Client, Reliable)
    void ClientShowLoadingScreen();

    // ============================================
    // Game Event Handlers
    // ============================================
    UFUNCTION()
    void OnMoneyCollectedChanged(int32 Current, int32 Target);

    void OnHealthChanged(const struct FOnAttributeChangeData& Data);
    void OnIllegalTagChanged(const struct FGameplayTag Tag, int32 NewCount);

    // ============================================
    // Server RPCs
    // ============================================
    UFUNCTION(Server, Reliable)
    void ServerTryLockPick(AActor* TargetDoor, float Angle);

private:
    // ============================================
    // Internal State
    // ============================================
    UPROPERTY()
    class ACharacterHUD* CharacterHUD;

    UPROPERTY()
    UPauseMenuWidget* PauseMenuWidget;

    FDelegateHandle IllegalTagDelegateHandle;

    // ============================================
    // Initialization & Binding
    // ============================================
    void BindAttributeDelegates();
    void BindGameplayTagDelegates();

    // ============================================
    // Pause Menu
    // ============================================
    UFUNCTION()
    void HandlePauseMenuResumed();

    // ============================================
    // Ping Update
    // ============================================
    FTimerHandle PingUpdateTimerHandle;
    void UpdatePingOnOverlay();

    // ============================================
    // Alarm System
    // ============================================
    UPROPERTY()
    float AlarmEndTime = 0.f;

    FTimerHandle AlarmUpdateTimerHandle;

    UFUNCTION()
    void HandleAlarmStarted(float InAlarmEndTime);

    UFUNCTION()
    void HandleAlarmCanceled();

    void UpdateAlarmUI();

    // ============================================
    // Pre-Alarm System
    // ============================================
    UPROPERTY()
    float PreAlarmEndTime = 0.f;

    FTimerHandle PreAlarmUpdateTimerHandle;

    UFUNCTION()
    void HandlePreAlarmStarted(float InPreAlarmEndTime, AActor* PreAlarmInstigator);

    UFUNCTION()
    void HandlePreAlarmCanceled();

    void UpdatePreAlarmUI();
};