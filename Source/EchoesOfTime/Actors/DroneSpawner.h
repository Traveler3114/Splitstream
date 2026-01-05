// DroneSpawner.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "Interfaces/IRepairable.h"
#include "TimelineEra.h"
#include "DataAssets/ItemBase.h"
#include "DroneSpawner.generated.h"

class UTextRenderComponent;
class ADronePawn;

UCLASS()
class ECHOESOFTIME_API ADroneSpawner : public AActor, public IInteractable, public IRepairable
{
	GENERATED_BODY()

public:
	ADroneSpawner();

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void CancelInteract_Implementation(AActor* Interactor) override;
	virtual void SetHighlighted_Implementation(bool bHighlight) override;
	virtual bool IsProgressiveInteract_Implementation() override { return true; }
	void OnSearchComplete();

	//virtual void RequestRepair_Implementation(AActor* RepairInstigator) override;
	

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Proximity Hack|Reward")
	UItemBase* RewardItem = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone")
	class USearchComponent* SearchComponent;
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone")
	UStaticMeshComponent* PrinterMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone")
	UStaticMeshComponent* PlatformMesh;

	UPROPERTY(VisibleAnywhere)
	UTextRenderComponent* CountdownText;

	// Config
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	ETimelineEra TimelineEra = ETimelineEra::Past;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TSubclassOf<ADronePawn> DroneClass;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	float RespawnDelay = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	float PlatformRiseOffset = 85.f;

	// State
	UPROPERTY()
	TArray<ADronePawn*> SpawnedDrones;

	UPROPERTY()
	ADronePawn* PendingDrone = nullptr;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	FVector DroneSpawnOffset = FVector(0.f, -10.f, 5.f);

	float RespawnTimeLeft = 0.0f;
	float PlatformStartZ;
	float TimerAnimElapsed = 0.0f;      // For up
	float PlatformDownAnimElapsed = 0.f; // For down

	bool bIsPlatformReverse = false; // false = up, true = down

	FTimerHandle RespawnTimerHandle;
	FTimerHandle TextUpdateTimerHandle;
	FTimerHandle PlatformAnimTimerHandle;

	// Spawning and animation
	UFUNCTION()
	void HandleDroneDeath(ADronePawn* DeadDrone);

	void BindToDroneDeath(ADronePawn* Drone);

	void ActivatePendingDrone();

	void UpdateCountdownText();
	void TickPlatformAnim();

	// Called after RespawnDelay
	void OnRespawnTimerFinished();

	// For safety, resets
	void ResetPlatform();
};