// DefaultGameState.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "DefaultGameState.generated.h"

class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAlarmStarted, float, AlarmEndTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAlarmCanceled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRestartRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPreAlarmStarted, float, PreAlarmEndTime, AActor*, PreAlarmInstigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPreAlarmCanceled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMoneyCollectedChanged, int32, CurrentMoney, int32, TargetMoney);

UCLASS()
class ECHOESOFTIME_API ADefaultGameState : public AGameState
{
	GENERATED_BODY()

public:
	ADefaultGameState();

	// ============================================
	// Unreal Engine Overrides
	// ============================================
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ============================================
	// Alarm System
	// ============================================
	UPROPERTY(ReplicatedUsing = OnRep_AlarmStarted, BlueprintReadOnly, Category = "Alarm")
	float AlarmEndTime;

	UPROPERTY(ReplicatedUsing = OnRep_AlarmActive, BlueprintReadOnly, Category = "Alarm")
	bool bAlarmActive;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Alarm")
	AActor* AlarmInstigator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alarm")
	float AlarmDuration = 5.f;

	UPROPERTY(BlueprintAssignable)
	FOnAlarmStarted OnAlarmStarted;

	UPROPERTY(BlueprintAssignable)
	FOnAlarmCanceled OnAlarmCanceled;

	UFUNCTION(BlueprintCallable)
	void StartAlarm(AActor* InAlarmInstigator = nullptr);

	UFUNCTION(BlueprintCallable)
	void CancelAlarm(AActor* InAlarmInstigator = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetRemainingAlarmTime() const;

	// ============================================
	// Pre-Alarm System
	// ============================================
	UPROPERTY(ReplicatedUsing = OnRep_PreAlarmStarted, BlueprintReadOnly, Category = "Alarm")
	float PreAlarmEndTime;

	UPROPERTY(ReplicatedUsing = OnRep_PreAlarmActive, BlueprintReadOnly, Category = "Alarm")
	bool bPreAlarmActive;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Alarm")
	AActor* PreAlarmInstigator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alarm")
	float PreAlarmDuration = 3.f;

	UPROPERTY(BlueprintAssignable)
	FOnPreAlarmStarted OnPreAlarmStarted;

	UPROPERTY(BlueprintAssignable)
	FOnPreAlarmCanceled OnPreAlarmCanceled;

	UFUNCTION(BlueprintCallable)
	void StartPreAlarm(AActor* InPreAlarmInstigator, float Duration);

	UFUNCTION(BlueprintCallable)
	void CancelPreAlarm(AActor* InPreAlarmInstigator = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetRemainingPreAlarmTime() const;

	// ============================================
	// Objective System
	// ============================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Objective")
	int32 TargetMoneyAmount = 50000;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentMoneyCollected, BlueprintReadOnly, Category = "Objective")
	int32 CurrentMoneyCollected = 0;

	UPROPERTY(BlueprintAssignable)
	FOnMoneyCollectedChanged OnMoneyCollectedChanged;

	UFUNCTION(BlueprintCallable)
	void AddCollectedMoney(int32 Amount);

	UFUNCTION()
	void OnRep_CurrentMoneyCollected();

	// ============================================
	// Game Control
	// ============================================
	UPROPERTY(BlueprintAssignable)
	FOnRestartRequested OnRestartRequested;

	UFUNCTION(BlueprintCallable)
	void RequestRestart();

protected:
	// ============================================
	// Replication Callbacks
	// ============================================
	UFUNCTION()
	void OnRep_AlarmStarted();

	UFUNCTION()
	void OnRep_AlarmActive();

	UFUNCTION()
	void OnRep_PreAlarmStarted();

	UFUNCTION()
	void OnRep_PreAlarmActive();
};