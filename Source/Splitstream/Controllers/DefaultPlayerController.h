// DefaultPlayerController.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Controllers/BasePlayerController.h"
#include "TimelineEra.h"
#include "DefaultPlayerController.generated.h"

class UPauseMenuWidget;
class ATerminal;

/** Tracks per-detector detection state for UI widget positioning. */
USTRUCT()
struct FTrackedDetection
{
    GENERATED_BODY()

    UPROPERTY()
    AActor* Detector = nullptr;

    float Progress = 0.f;
    bool bIsLocked = false;
};

/**
 * Player controller for the default heist game mode.
 *
 * Manages HUD overlay setup, pause menu, alarm/pre-alarm UI updates,
 * detection widget positioning, guard repair countdown display,
 * server-side action forwarding, and GAS delegate binding for health
 * and illegal tag state changes.
 */
UCLASS()
class SPLITSTREAM_API ADefaultPlayerController : public ABasePlayerController
{
    GENERATED_BODY()
public:
    ADefaultPlayerController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void SetupInputComponent() override;
    virtual void OnRep_PlayerState() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    float GetSyncedServerTime() const;

    UFUNCTION(Client, Reliable)
    void ClientEnterSpectatorMode();

    void SetupOverlay();

    void TogglePauseMenu();

    void BindAttributeDelegates();
    void BindGameplayTagDelegates();

    void OnHealthChanged(const struct FOnAttributeChangeData& Data);
    void OnIllegalTagChanged(const struct FGameplayTag Tag, int32 NewCount);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UPauseMenuWidget> PauseMenuWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UCalendarWidget> CalendarWidgetClass;

    UPROPERTY(BlueprintReadWrite)
    UCalendarWidget* CalendarWidgetInstance;

    bool bIsPauseMenuOpen = false;

    UFUNCTION(Client, Reliable)
    void ClientShowCalendarWidget(const TArray<FCalendarDateRecord>& CivilianDateRecords);

    UFUNCTION(Client, Unreliable)
    void ClientUpdateDetectionWidget(AActor* Detector, float Progress, bool bIsLocked);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void OnMoneyCollectedChanged(int32 Current, int32 Target);

    UFUNCTION()
    void HandleRepairETAStarted(AActor* Repairable, float Duration);

    UFUNCTION(Server, Reliable)
    void ServerExecuteAction(UObject* Target, const FServerActionPayload& Payload);
private:
    TSet<AActor*> LastFrameDetectedActors;

    UPROPERTY()
    class ACharacterHUD* CharacterHUD;

    UPROPERTY()
    UPauseMenuWidget* PauseMenuWidget;

    FDelegateHandle IllegalTagDelegateHandle;

    UFUNCTION()
    void HandlePauseMenuResumed();

    FTimerHandle PingUpdateTimerHandle;
    void UpdatePingOnOverlay();

    UPROPERTY()
    float AlarmEndTime = 0.f;

    FTimerHandle AlarmUpdateTimerHandle;

    UFUNCTION()
    void HandleAlarmStarted(float InAlarmEndTime, ETimelineEra Era);

    UFUNCTION()
    void HandleAlarmCanceled();

    void UpdateAlarmUI();

    UPROPERTY()
    float PreAlarmEndTime = 0.f;

    FTimerHandle PreAlarmUpdateTimerHandle;

    UFUNCTION()
    void HandlePreAlarmStarted(float InPreAlarmEndTime, AActor* PreAlarmInstigator, ETimelineEra Era);

    UFUNCTION()
    void HandlePreAlarmCanceled(ETimelineEra Era);

    void UpdatePreAlarmUI();

    FTimerHandle RepairCountdownsUpdateTimer;
    TMap<AActor*, float> ActiveRepairCountdowns;

    void UpdateRepairCountdownUI();

    // --- Detection widget tracking for per-frame position updates ---
    UPROPERTY()
    TMap<AActor*, FTrackedDetection> TrackedDetections;

    /** Computes screen position and on-screen flag for a detector actor. */
    void ComputeDetectorScreenPosition(AActor* Detector, FVector2D& OutScreenPos, bool& bOutIsOnScreen) const;

    /** Called every frame to update all tracked detection widget positions. */
    void UpdateAllDetectionWidgetPositions();
};