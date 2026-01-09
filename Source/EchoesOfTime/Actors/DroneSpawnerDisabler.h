// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimelineEra.h"
#include "Interfaces/IInteractable.h"
#include "ActorComponents/SearchComponent.h"
#include "DroneSpawnerDisabler.generated.h"

UCLASS()
class ECHOESOFTIME_API ADroneSpawnerDisabler : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADroneSpawnerDisabler();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETimelineEra TimelineEra = ETimelineEra::Past;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device")
	class USearchComponent* SearchComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	UStaticMeshComponent* DeviceMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Device")
	TArray<AActor*> CompletionTargets;

	// Meshes for different timeline eras (set in Blueprints)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device")
	UStaticMesh* PastMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device")
	UStaticMesh* FutureMesh;

	void SetMesh();

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnSearchComplete();

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void CancelInteract_Implementation(AActor* Interactor) override;
	virtual bool IsProgressiveInteract_Implementation() override { return true; }
	virtual void SetHighlighted_Implementation(bool bHighlighted) override;

private:
	UPROPERTY(Replicated)
	bool bVisualOnly = false;

};