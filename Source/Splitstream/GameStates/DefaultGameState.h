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
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPreAlarmCanceled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMoneyCollectedChanged, int32, CurrentMoney, int32, TargetMoney);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGuardRepairETAStarted, AActor*, TargetActor, float, Duration);

USTRUCT(BlueprintType)
struct FGuardRepairCountdown
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    AActor* TargetActor = nullptr;

    UPROPERTY(BlueprintReadWrite)
    float ETA = 0.f;
};

USTRUCT()
struct FPreAlarmInstigatorInfo
{
    GENERATED_BODY()

    UPROPERTY()
    AActor* Instigator = nullptr;

    UPROPERTY()
    float ETA = 0.f;

    UPROPERTY()
    ETimelineEra Era = ETimelineEra::Past;

    FPreAlarmInstigatorInfo() : Instigator(nullptr), ETA(0.f), Era(ETimelineEra::Past) {}
    FPreAlarmInstigatorInfo(AActor* InActor, float InETA, ETimelineEra InEra)
        : Instigator(InActor), ETA(InETA), Era(InEra) {
    }
};

USTRUCT()
struct FPerEraPreAlarmState
{
    GENERATED_BODY()

    UPROPERTY()
    float EndTime = 0.f;

    UPROPERTY()
    bool bActive = false;

    UPROPERTY()
    AActor* SoonestInstigator = nullptr;

    UPROPERTY()
    TArray<FPreAlarmInstigatorInfo> InstigatorsInfo;
};

UCLASS()
class SPLITSTREAM_API ADefaultGameState : public ABaseGameState
{
    GENERATED_BODY()

public:
    ADefaultGameState();

    // ALARM
    UPROPERTY(ReplicatedUsing = OnRep_AlarmStarted, BlueprintReadOnly, Category = "Alarm")
    float AlarmEndTime;
    UPROPERTY(ReplicatedUsing = OnRep_AlarmActive, BlueprintReadOnly, Category = "Alarm")
    bool bAlarmActive;
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Alarm")
    AActor* AlarmInstigator;
    UPROPERTY(Replicated)
    ETimelineEra AlarmEra = ETimelineEra::Past;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alarm")
    float AlarmDuration = 5.f;

    UPROPERTY(BlueprintAssignable)
    FOnAlarmStarted OnAlarmStarted;
    UPROPERTY(BlueprintAssignable)
    FOnAlarmCanceled OnAlarmCanceled;
    UPROPERTY(BlueprintAssignable)
    FOnRestartRequested OnRestartRequested;

    // PRE-ALARM (per era)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alarm")
    float PreAlarmDuration = 3.f;

    UPROPERTY(Replicated)
    FPerEraPreAlarmState PastPreAlarm;

    UPROPERTY(Replicated)
    FPerEraPreAlarmState FuturePreAlarm;

    UPROPERTY(BlueprintAssignable)
    FOnPreAlarmStarted OnPreAlarmStarted;

    UPROPERTY(BlueprintAssignable)
    FOnPreAlarmCanceled OnPreAlarmCanceled;

    // GUARD REPAIR
    UPROPERTY(ReplicatedUsing = OnRep_GuardRepairCountdowns)
    TArray<FGuardRepairCountdown> GuardRepairCountdowns;

    UPROPERTY(BlueprintAssignable)
    FOnGuardRepairETAStarted OnGuardRepairETAStarted;

    // OBJECTIVE
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Objective")
    int32 TargetMoneyAmount = 50000;
    UPROPERTY(ReplicatedUsing = OnRep_CurrentMoneyCollected, BlueprintReadOnly, Category = "Objective")
    int32 CurrentMoneyCollected = 0;

    UPROPERTY(BlueprintAssignable)
    FOnMoneyCollectedChanged OnMoneyCollectedChanged;

    // --- API ---
    UFUNCTION(BlueprintCallable)
    void StartGuardRepairCountdown(AActor* Repairable, float Duration);

    UFUNCTION(BlueprintCallable)
    void StartAlarm(AActor* InAlarmInstigator, ETimelineEra Era);

    UFUNCTION(BlueprintCallable)
    void CancelAlarm(AActor* InAlarmInstigator = nullptr);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    float GetRemainingAlarmTime() const;

    UFUNCTION(BlueprintCallable)
    void RequestRestart();

    UFUNCTION(BlueprintCallable)
    void StartPreAlarm(AActor* InPreAlarmInstigator, float Duration, ETimelineEra Era);

    UFUNCTION(BlueprintCallable)
    void CancelPreAlarm(AActor* InCancelingInstigator = nullptr, ETimelineEra Era = ETimelineEra::Past);

    UFUNCTION(BlueprintCallable)
    void RemovePreAlarmInstigator(AActor* InToRemoveInstigator, ETimelineEra Era);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    float GetRemainingPreAlarmTime(ETimelineEra Era) const;

    UFUNCTION(BlueprintCallable)
    void AddCollectedMoney(int32 Amount);

protected:
    UFUNCTION()
    void OnRep_AlarmStarted();

    UFUNCTION()
    void OnRep_AlarmActive();

    UFUNCTION()
    void OnRep_PreAlarmStarted();

    UFUNCTION()
    void OnRep_PreAlarmActive();

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