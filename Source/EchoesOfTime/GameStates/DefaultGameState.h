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

UCLASS()
class ECHOESOFTIME_API ADefaultGameState : public AGameState
{
	GENERATED_BODY()

public:
	ADefaultGameState();

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

	UPROPERTY(BlueprintAssignable)
	FOnRestartRequested OnRestartRequested;

	// --- PRE-ALARM ---
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

	// ALARM
	UFUNCTION(BlueprintCallable)
	void StartAlarm(AActor* InAlarmInstigator = nullptr);

	UFUNCTION(BlueprintCallable)
	void CancelAlarm(AActor* InAlarmInstigator = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetRemainingAlarmTime() const;

	UFUNCTION(BlueprintCallable)
	void RequestRestart();

	// PRE-ALARM
	UFUNCTION(BlueprintCallable)
	void StartPreAlarm(AActor* InPreAlarmInstigator, float Duration);

	UFUNCTION(BlueprintCallable)
	void CancelPreAlarm(AActor* InPreAlarmInstigator = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetRemainingPreAlarmTime() const;

	// Amount of money players need to collect to complete the objective
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Objective")
	int32 TargetMoneyAmount = 50000;

	// Current progress (total money collected so far)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Objective")
	int32 CurrentMoneyCollected = 0;

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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};