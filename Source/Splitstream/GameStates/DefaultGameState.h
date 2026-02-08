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
    ETimelineEra Era = ETimelineEra::Past; // Add this

    FPreAlarmInstigatorInfo() : Instigator(nullptr), ETA(0.f), Era(ETimelineEra::Past) {}
    FPreAlarmInstigatorInfo(AActor* InActor, float InETA, ETimelineEra InEra)
        : Instigator(InActor), ETA(InETA), Era(InEra) {
    }
};

UCLASS()
class SPLITSTREAM_API ADefaultGameState : public ABaseGameState
{
    GENERATED_BODY()

public:
    ADefaultGameState();

    UPROPERTY(ReplicatedUsing = OnRep_AlarmStarted, BlueprintReadOnly, Category = "Alarm")
    float AlarmEndTime;

    UPROPERTY(Replicated)
    ETimelineEra AlarmEra = ETimelineEra::Past;

    UPROPERTY(ReplicatedUsing = OnRep_AlarmActive, BlueprintReadOnly, Category = "Alarm")
    bool bAlarmActive;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Alarm")
    AActor* AlarmInstigator;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alarm")
    float AlarmDuration = 5.f;

    UPROPERTY(ReplicatedUsing = OnRep_GuardRepairCountdowns)
    TArray<FGuardRepairCountdown> GuardRepairCountdowns;

    UPROPERTY(BlueprintAssignable)
    FOnGuardRepairETAStarted OnGuardRepairETAStarted;

    UFUNCTION(BlueprintCallable)
    void StartGuardRepairCountdown(AActor* Repairable, float Duration);

    UPROPERTY(BlueprintAssignable)
    FOnAlarmStarted OnAlarmStarted;

    UPROPERTY(BlueprintAssignable)
    FOnAlarmCanceled OnAlarmCanceled;

    UPROPERTY(BlueprintAssignable)
    FOnRestartRequested OnRestartRequested;

    // --- PRE-ALARM ---
    UPROPERTY(ReplicatedUsing = OnRep_PreAlarmStarted, BlueprintReadOnly, Category = "Alarm")
    float PreAlarmEndTime;

    UPROPERTY(ReplicatedUsing = OnRep_PreAlarmActive, BlueprintReadOnly, Category = "Alarm")
    bool bPreAlarmActive;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alarm")
    float PreAlarmDuration = 3.f;

    // Structs to represent each instigator and their individual ETA.
    UPROPERTY(Replicated)
    TArray<FPreAlarmInstigatorInfo> PreAlarmInstigatorsInfo;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Alarm")
    AActor* PreAlarmSoonestInstigator;

    UPROPERTY(BlueprintAssignable)
    FOnPreAlarmStarted OnPreAlarmStarted;

    UPROPERTY(BlueprintAssignable)
    FOnPreAlarmCanceled OnPreAlarmCanceled;

    // ALARM
    UFUNCTION(BlueprintCallable)
    void StartAlarm(AActor* InAlarmInstigator, ETimelineEra Era);

    UFUNCTION(BlueprintCallable)
    void CancelAlarm(AActor* InAlarmInstigator = nullptr);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    float GetRemainingAlarmTime() const;

    UFUNCTION(BlueprintCallable)
    void RequestRestart();

    // PRE-ALARM
    UFUNCTION(BlueprintCallable)
    void StartPreAlarm(AActor* InPreAlarmInstigator, float Duration, ETimelineEra Era);

    UFUNCTION(BlueprintCallable)
    void CancelPreAlarm(AActor* InCancelingInstigator = nullptr);

    UFUNCTION(BlueprintCallable)
    void RemovePreAlarmInstigator(AActor* InToRemoveInstigator);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    float GetRemainingPreAlarmTime() const;

    // Amount of money players need to collect to complete the objective
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Objective")
    int32 TargetMoneyAmount = 50000;

    // Current progress (total money collected so far)
    UPROPERTY(ReplicatedUsing = OnRep_CurrentMoneyCollected, BlueprintReadOnly, Category = "Objective")
    int32 CurrentMoneyCollected = 0;

    UPROPERTY(BlueprintAssignable)
    FOnMoneyCollectedChanged OnMoneyCollectedChanged;

    UFUNCTION()
    void OnRep_CurrentMoneyCollected();

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
    void OnRep_GuardRepairCountdowns();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Helper
    void UpdatePreAlarmSoonestInstigator();
};