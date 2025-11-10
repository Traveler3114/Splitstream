// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MetalDetector.generated.h"

UCLASS()
class ECHOESOFTIME_API AMetalDetector : public AActor
{
	GENERATED_BODY()

public:
	AMetalDetector();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Overlap event
	UFUNCTION()
	void OnPlayerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	// RepNotify called when 'bEnabled' changes
	UFUNCTION()
	void OnRep_Enabled();


private:
	// Mesh for the detector
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* DetectorMesh;

	// Collision box for overlap detection
	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* TriggerBox;

	// Replicated enabled state
	UPROPERTY(ReplicatedUsing = OnRep_Enabled)
	bool bEnabled = true;
};