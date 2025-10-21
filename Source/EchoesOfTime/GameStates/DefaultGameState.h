// DefaultGameState.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "DefaultGameState.generated.h"

class AActor; // forward declaration to avoid heavy includes

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAlarmStarted, float, AlarmEndTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAlarmCanceled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRestartRequested);

UCLASS()
class ECHOESOFTIME_API ADefaultGameState : public AGameState
{
	GENERATED_BODY()

public:
	ADefaultGameState();

	// Replicated absolute server time when the alarm ends (GetWorld()->GetTimeSeconds() reference)
	UPROPERTY(ReplicatedUsing = OnRep_AlarmStarted, BlueprintReadOnly, Category = "Alarm")
	float AlarmEndTime;

	UPROPERTY(ReplicatedUsing = OnRep_AlarmActive, BlueprintReadOnly, Category = "Alarm")
	bool bAlarmActive;

	// Actor that started the current alarm (may be nullptr for non-actor sources)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Alarm")
	AActor* AlarmInstigator;

	// Default duration used when server starts alarm (server-only editable)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alarm")
	float AlarmDuration = 5.f;

	// Delegate to notify clients that an alarm has started (delivers absolute AlarmEndTime)
	UPROPERTY(BlueprintAssignable)
	FOnAlarmStarted OnAlarmStarted;

	// Delegate to notify clients alarm was canceled
	UPROPERTY(BlueprintAssignable)
	FOnAlarmCanceled OnAlarmCanceled;

	// Existing restart delegate kept
	UPROPERTY(BlueprintAssignable)
	FOnRestartRequested OnRestartRequested;

	// Server-callable to start the alarm (server-side authoritative). InAlarmInstigator may be nullptr.
	UFUNCTION(BlueprintCallable)
	void StartAlarm(AActor* InAlarmInstigator = nullptr);

	// Cancel the active alarm. If InAlarmInstigator != nullptr, only cancels if it matches AlarmInstigator.
	// If InAlarmInstigator == nullptr, performs a forced cancel.
	UFUNCTION(BlueprintCallable)
	void CancelAlarm(AActor* InAlarmInstigator = nullptr);

	// Helper to compute remaining time locally from this object's GetWorld()->GetTimeSeconds()
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetRemainingAlarmTime() const;

	// Existing RequestRestart kept for compatibility
	UFUNCTION(BlueprintCallable)
	void RequestRestart();

protected:
	UFUNCTION()
	void OnRep_AlarmStarted();

	UFUNCTION()
	void OnRep_AlarmActive();

	// replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};