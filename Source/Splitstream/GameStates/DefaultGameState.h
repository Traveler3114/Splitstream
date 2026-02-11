#pragma once

#include "CoreMinimal.h"
#include "BaseGameState.h"
#include "TimelineEra.h"
#include "DefaultGameState.generated.h"

class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAlarmStarted, float, AlarmEndTime, ETimelineEra, Era);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAlarmCanceled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRestartRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPreAlarmStarted, float, PreAlarmEndTime, AActor*, PreAlarmInstigator, ETimelineEra, Era);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPreAlarmCanceled, ETimelineEra, Era);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMoneyCollectedChanged, int32, CurrentMoney, int32, TargetMoney);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGuardRepairETAStarted, AActor*, TargetActor, float, Duration);

/** Tracks a pending guard repair with target actor and estimated completion time. */
USTRUCT(BlueprintType)
struct FGuardRepairCountdown
{
    GENERATED_BODY()

    /** The repairable actor being targeted for repair. */
    UPROPERTY(BlueprintReadWrite)
    AActor* TargetActor = nullptr;

    /** Estimated time until repair is complete (server time). */
    UPROPERTY(BlueprintReadWrite)
    float ETA = 0.f;
};

/** Tracks a single pre-alarm instigator and its countdown ETA within a specific era. */
USTRUCT()
struct FPreAlarmInstigatorInfo
{
    GENERATED_BODY()

    /** The actor that caused this pre-alarm entry (e.g., a guard or camera). */
    UPROPERTY()
    AActor* Instigator = nullptr;

    /** Server time at which this pre-alarm expires and escalates. */
    UPROPERTY()
    float ETA = 0.f;

    /** The era this pre-alarm belongs to. */
    UPROPERTY()
    ETimelineEra Era = ETimelineEra::Past;

    FPreAlarmInstigatorInfo() : Instigator(nullptr), ETA(0.f), Era(ETimelineEra::Past) {}
    FPreAlarmInstigatorInfo(AActor* InActor, float InETA, ETimelineEra InEra)
        : Instigator(InActor), ETA(InETA), Era(InEra) {
    }
};

/** Replicated pre-alarm state for a single timeline era (Past or Future). */
USTRUCT()
struct FPerEraPreAlarmState
{
    GENERATED_BODY()

    /** Server time at which the soonest pre-alarm expires. */
    UPROPERTY()
    float EndTime = 0.f;

    /** Whether a pre-alarm is currently active in this era. */
    UPROPERTY()
    bool bActive = false;

    /** The instigator with the soonest ETA in this era. */
    UPROPERTY()
    AActor* SoonestInstigator = nullptr;

    /** All active pre-alarm instigators and their ETAs for this era. */
    UPROPERTY()
    TArray<FPreAlarmInstigatorInfo> InstigatorsInfo;
};

/**
 * Core game state for the default heist gameplay mode.
 *
 * Manages the alarm and pre-alarm systems (per-era), guard repair countdowns,
 * and the money collection objective. All state is replicated to clients and
 * exposed via Blueprint-assignable delegates for UI binding.
 *
 * The alarm system is server-authoritative: only the server can start, cancel,
 * or modify alarm/pre-alarm state. Clients receive updates via OnRep callbacks.
 */
UCLASS()
class SPLITSTREAM_API ADefaultGameState : public ABaseGameState
{
    GENERATED_BODY()

public:
    ADefaultGameState();

    // ---- ALARM STATE ----

    /** Server time at which the current alarm expires. Replicated. */
    UPROPERTY(ReplicatedUsing = OnRep_AlarmStarted, BlueprintReadOnly, Category = "Alarm")
    float AlarmEndTime;

    /** Whether an alarm is currently active. Replicated. */
    UPROPERTY(ReplicatedUsing = OnRep_AlarmActive, BlueprintReadOnly, Category = "Alarm")
    bool bAlarmActive;

    /** The actor that triggered the current alarm (e.g., a security camera). Replicated. */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Alarm")
    AActor* AlarmInstigator;

    /** The timeline era in which the alarm was triggered. Replicated. */
    UPROPERTY(Replicated)
    ETimelineEra AlarmEra = ETimelineEra::Past;

    /** How long (seconds) an alarm lasts once triggered. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alarm")
    float AlarmDuration = 5.f;

    /** Broadcast when an alarm starts. Provides the end time and era. */
    UPROPERTY(BlueprintAssignable)
    FOnAlarmStarted OnAlarmStarted;

    /** Broadcast when an active alarm is canceled. */
    UPROPERTY(BlueprintAssignable)
    FOnAlarmCanceled OnAlarmCanceled;

    /** Broadcast when a game restart is requested (e.g., alarm expired). */
    UPROPERTY(BlueprintAssignable)
    FOnRestartRequested OnRestartRequested;

    // ---- PRE-ALARM STATE (per era) ----

    /** How long (seconds) a pre-alarm countdown lasts before escalating to a full alarm. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alarm")
    float PreAlarmDuration = 3.f;

    /** Pre-alarm state for the Past era. Replicated. */
    UPROPERTY(ReplicatedUsing = OnRep_PastPreAlarm)
    FPerEraPreAlarmState PastPreAlarm;

    /** Pre-alarm state for the Future era. Replicated. */
    UPROPERTY(ReplicatedUsing = OnRep_FuturePreAlarm)
    FPerEraPreAlarmState FuturePreAlarm;

    /** Broadcast when a pre-alarm starts or is updated. Provides end time, instigator, and era. */
    UPROPERTY(BlueprintAssignable)
    FOnPreAlarmStarted OnPreAlarmStarted;

    /** Broadcast when a pre-alarm is fully canceled for an era. */
    UPROPERTY(BlueprintAssignable)
    FOnPreAlarmCanceled OnPreAlarmCanceled;

    // ---- GUARD REPAIR ----

    /** Active guard repair countdowns, replicated for UI display. */
    UPROPERTY(ReplicatedUsing = OnRep_GuardRepairCountdowns)
    TArray<FGuardRepairCountdown> GuardRepairCountdowns;

    /** Broadcast when a guard begins traveling to repair a target. */
    UPROPERTY(BlueprintAssignable)
    FOnGuardRepairETAStarted OnGuardRepairETAStarted;

    // ---- OBJECTIVE ----

    /** Total money required to complete the heist objective. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Objective")
    int32 TargetMoneyAmount = 50000;

    /** Current money collected by all players. Replicated. */
    UPROPERTY(ReplicatedUsing = OnRep_CurrentMoneyCollected, BlueprintReadOnly, Category = "Objective")
    int32 CurrentMoneyCollected = 0;

    /** Broadcast when the collected money amount changes. */
    UPROPERTY(BlueprintAssignable)
    FOnMoneyCollectedChanged OnMoneyCollectedChanged;

    // --- PUBLIC API ---

    /**
     * Starts a guard repair countdown for the given repairable actor.
     * @param Repairable  The actor being repaired.
     * @param Duration    Estimated time until repair completes.
     */
    UFUNCTION(BlueprintCallable)
    void StartGuardRepairCountdown(AActor* Repairable, float Duration);

    /**
     * Starts a full alarm. Server-only.
     * Cancels any active pre-alarm in the same era.
     * @param InAlarmInstigator  The actor that triggered the alarm.
     * @param Era                The timeline era where the alarm occurred.
     */
    UFUNCTION(BlueprintCallable)
    void StartAlarm(AActor* InAlarmInstigator, ETimelineEra Era);

    /**
     * Cancels the active alarm. Server-only.
     * If InAlarmInstigator is specified, the alarm is only canceled if it matches the current instigator.
     * @param InAlarmInstigator  Optional: only cancel if this matches the current alarm instigator.
     */
    UFUNCTION(BlueprintCallable)
    void CancelAlarm(AActor* InAlarmInstigator = nullptr);

    /**
     * Returns the remaining alarm time in seconds. Returns 0 if no alarm is active.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure)
    float GetRemainingAlarmTime() const;

    /** Broadcasts a restart request to all listeners. */
    UFUNCTION(BlueprintCallable)
    void RequestRestart();

    /**
     * Starts or updates a pre-alarm for the given instigator and era. Server-only.
     * Multiple instigators can have concurrent pre-alarms; the soonest ETA is used.
     * @param InPreAlarmInstigator  The actor causing the pre-alarm (e.g., a guard).
     * @param Duration              Pre-alarm countdown duration in seconds.
     * @param Era                   The timeline era for this pre-alarm.
     */
    UFUNCTION(BlueprintCallable)
    void StartPreAlarm(AActor* InPreAlarmInstigator, float Duration, ETimelineEra Era);

    /**
     * Cancels the pre-alarm for the given era. Server-only.
     * If InCancelingInstigator is null, all pre-alarm instigators in that era are cleared.
     * Otherwise, only the specified instigator is removed.
     * @param InCancelingInstigator  Optional: only remove this specific instigator.
     * @param Era                    The timeline era to cancel the pre-alarm for.
     */
    UFUNCTION(BlueprintCallable)
    void CancelPreAlarm(AActor* InCancelingInstigator = nullptr, ETimelineEra Era = ETimelineEra::Past);

    /**
     * Removes a single pre-alarm instigator from the specified era. Server-only.
     * If no instigators remain, the pre-alarm is fully canceled.
     * @param InToRemoveInstigator  The instigator to remove.
     * @param Era                   The timeline era to modify.
     */
    UFUNCTION(BlueprintCallable)
    void RemovePreAlarmInstigator(AActor* InToRemoveInstigator, ETimelineEra Era);

    /**
     * Returns the remaining pre-alarm time (seconds) for the given era.
     * Returns 0 if no pre-alarm is active in that era.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure)
    float GetRemainingPreAlarmTime(ETimelineEra Era) const;

    /**
     * Adds money to the collected total. Server-only. Clamped to TargetMoneyAmount.
     * @param Amount  The amount of money to add.
     */
    UFUNCTION(BlueprintCallable)
    void AddCollectedMoney(int32 Amount);

protected:
    UFUNCTION()
    void OnRep_AlarmStarted();

    UFUNCTION()
    void OnRep_AlarmActive();

    UFUNCTION()
    void OnRep_PastPreAlarm();

    UFUNCTION()
    void OnRep_FuturePreAlarm();

    UFUNCTION()
    void OnRep_CurrentMoneyCollected();

    UFUNCTION()
    void OnRep_GuardRepairCountdowns();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Helpers for era
    FPerEraPreAlarmState& GetEraPreAlarm(ETimelineEra Era)
    {
        return (Era == ETimelineEra::Past) ? PastPreAlarm : FuturePreAlarm;
    }
    const FPerEraPreAlarmState& GetEraPreAlarm(ETimelineEra Era) const
    {
        return (Era == ETimelineEra::Past) ? PastPreAlarm : FuturePreAlarm;
    }
};