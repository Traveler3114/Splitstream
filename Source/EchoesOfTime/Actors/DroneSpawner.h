#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DroneSpawner.generated.h"

class UTextRenderComponent;
class ADronePawn;

UCLASS()
class ECHOESOFTIME_API ADroneSpawner : public AActor
{
	GENERATED_BODY()

public:
	ADroneSpawner();

protected:
	virtual void BeginPlay() override;

	// Handle when a drone dies
	UFUNCTION()
	void HandleDroneDeath(ADronePawn* DeadDrone);

	// Helper to bind delegate
	void BindToDroneDeath(ADronePawn* Drone);

	// Spawns a new drone
	void SpawnDrone();

	// Store spawned drones for later tracking if needed
	UPROPERTY()
	TArray<ADronePawn*> SpawnedDrones;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone")
	UStaticMeshComponent* PrinterMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone")
	UStaticMeshComponent* PlatformMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone")
	UStaticMeshComponent* OpeningMesh;

	UPROPERTY(VisibleAnywhere)
	UTextRenderComponent* CountdownText;

	// Time between death and respawn, configurable in editor
	UPROPERTY(EditAnywhere, Category = "Spawning")
	float RespawnDelay = 5.0f;

	// Time left until spawn
	float RespawnTimeLeft = 0.0f;

	// Timer to trigger respawn
	FTimerHandle RespawnTimerHandle;

	// Timer to update the text every second
	FTimerHandle TextUpdateTimerHandle;

	// Called repeatedly to update countdown text
	void UpdateCountdownText();

	// Called after RespawnDelay to actually spawn the drone
	void OnRespawnTimerFinished();
};