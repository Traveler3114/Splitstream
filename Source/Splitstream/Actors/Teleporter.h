// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "DataAssets/ItemBase.h"
#include "TimelineEra.h"
#include "Teleporter.generated.h"

UCLASS()
class SPLITSTREAM_API ATeleporter : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	ATeleporter();
	void UpdateTeleporterColour();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locker")
	ETimelineEra TimelineEra = ETimelineEra::Future;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* TeleporterMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UBoxComponent* TeleportVolume;

	// Arrow to set teleport destination & direction
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UArrowComponent* TeleportTarget;

	// Reference to the teleporter this one links to
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Teleporter")
	ATeleporter* OtherTeleporter;

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void SetHighlighted_Implementation(bool bHighlight) override;
	virtual bool IsCorrectItem_Implementation(const FInventorySlot& Slot) const override;
	virtual bool RequiresItem_Implementation() const override { return true; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadScanner")
	EItemType RequiredItem = EItemType::PowerCell;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = false;

	UFUNCTION()
	void OnTeleportVolumeBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};