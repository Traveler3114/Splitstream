// DroneSpawner.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "Interfaces/IRepairable.h"
#include "Interfaces/IPuzzleCompletionReceiver.h"
#include "TimelineEra.h"
#include "DataAssets/ItemBase.h"
#include "DroneSpawner.generated.h"



class UTextRenderComponent;
class ADronePawn;

UCLASS()
class SPLITSTREAM_API ADroneSpawner : public AActor, public IInteractable, public IRepairable, public IPuzzleCompletionReceiver
{
	GENERATED_BODY()

public:
	ADroneSpawner();

	float LastPlatformAnimTime = 0.f;

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void CancelInteract_Implementation(AActor* Interactor) override;
	virtual void SetHighlighted_Implementation(bool bHighlight) override;
	virtual bool IsProgressiveInteract_Implementation() override { return true; }
	UFUNCTION()
	void OnSearchComplete();

	// IRepairable interface required overrides
	virtual void RequestRepair_Implementation(AActor* RepairInstigator) override;
	virtual float GetRepairTime_Implementation() const override { return RepairTime; }
	virtual ETimelineEra GetTimelineEra_Implementation() const override { return TimelineEra; }
	virtual AActor* GetCompletionTarget_Implementation() const override { return const_cast<ADroneSpawner*>(this); }
	virtual FOnRepairRequested& GetOnRepairRequested() override { return OnRepairRequested; }

	virtual void OnPuzzleCompleted_Implementation() override;

	UPROPERTY(BlueprintAssignable, Category = "Repair")
	FOnRepairRequested OnRepairRequested;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair")
	float RepairTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Proximity Hack|Reward")
	UItemBase* RewardItem = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone")
	class USearchComponent* SearchComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone")
	UStaticMeshComponent* PrinterMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone")
	UStaticMeshComponent* PlatformMesh;

	UPROPERTY(VisibleAnywhere)
	UTextRenderComponent* CountdownText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	ETimelineEra TimelineEra = ETimelineEra::Past;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TSubclassOf<ADronePawn> DroneClass;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	float RespawnDelay = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	float PlatformRiseOffset = 85.f;

	// Make the array editable per instance in the editor, and in Blueprints
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Drone")
	TArray<ADronePawn*> SpawnedDrones;

	UPROPERTY()
	ADronePawn* PendingDrone = nullptr;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	FVector DroneSpawnOffset = FVector(0.f, -10.f, 5.f);

	float RespawnTimeLeft = 0.0f;
	float PlatformStartZ = 0.0f;
	float TimerAnimElapsed = 0.0f;
	float PlatformDownAnimElapsed = 0.f;

	bool bIsPlatformReverse = false;

	FTimerHandle RespawnTimerHandle;
	FTimerHandle TextUpdateTimerHandle;
	FTimerHandle PlatformAnimTimerHandle;

	//** NEW: PAUSE/RESUME STATE
	bool bPausedFromSearchOrDestroy = false;
	float PausedRespawnTimeLeft = 0.0f;
	float PausedTimerAnimElapsed = 0.0f;
	float PausedPlatformDownAnimElapsed = 0.0f;

	// Respawn queue
	int32 PendingSpawnCount = 0;

	// Spawning and animation
	UFUNCTION()
	void HandleDroneDeath(ADronePawn* DeadDrone);

	void BindToDroneDeath(ADronePawn* Drone);

	void ActivatePendingDrone();

	// NEW: Respawn queue support
	void StartNextPendingSpawn();

	UFUNCTION()
	void UpdateCountdownText();

	UFUNCTION()
	void TickPlatformAnim();

	UFUNCTION()
	void OnRespawnTimerFinished();

	void ResetPlatform();

	//** NEW: Pause/resume helpers
	void PauseAllTimers();
	void ResumeAllTimers();
};