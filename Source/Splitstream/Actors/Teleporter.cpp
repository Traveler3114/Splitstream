// Fill out your copyright notice in the Description page of Project Settings.

#include "Teleporter.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/Character.h"

// Sets default values
ATeleporter::ATeleporter()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	TeleporterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TeleporterMesh"));
	TeleporterMesh->SetupAttachment(SceneRoot);

	TeleportVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TeleportVolume"));
	TeleportVolume->SetupAttachment(SceneRoot);
	TeleportVolume->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	TeleportVolume->SetCollisionProfileName("Trigger");

	TeleportTarget = CreateDefaultSubobject<UArrowComponent>(TEXT("TeleportTarget"));
	TeleportTarget->SetupAttachment(SceneRoot);
}

void ATeleporter::BeginPlay()
{
	Super::BeginPlay();

	if (TeleportVolume)
	{
		TeleportVolume->OnComponentBeginOverlap.AddDynamic(this, &ATeleporter::OnTeleportVolumeBeginOverlap);
	}

	UpdateTeleporterColour(); // << Set correct colour at start
}

void ATeleporter::UpdateTeleporterColour()
{
	if (!TeleporterMesh) return;

	// Ensure we use a dynamic material instance, so we can change parameters at runtime
	UMaterialInstanceDynamic* DynMat = TeleporterMesh->CreateAndSetMaterialInstanceDynamic(0);
	if (!DynMat) return;

	FLinearColor OldColour = FLinearColor::White; // Default fallback

	// Read the OldColour from the material
	DynMat->GetVectorParameterValue(FName("OldColour"), OldColour);

	// Decide the NewColour value
	FLinearColor NewColour = bIsActive ? OldColour : FLinearColor::Red;

	// Always set OldColour to the value we originally get (never change at runtime)
	DynMat->SetVectorParameterValue(FName("OldColour"), OldColour);
	DynMat->SetVectorParameterValue(FName("NewColour"), NewColour);
}

void ATeleporter::OnTeleportVolumeBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	ACharacter* OverlappingCharacter = Cast<ACharacter>(OtherActor);
	if (!OverlappingCharacter) return;
	if (!OtherTeleporter) return;
	if (!HasAuthority()) return;
	if (!bIsActive || !OtherTeleporter->bIsActive) return;

	// Deactivate both BEFORE teleporting to avoid recursion
	bIsActive = false;
	OtherTeleporter->bIsActive = false;

	if (TeleportVolume)
		TeleportVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (OtherTeleporter->TeleportVolume)
		OtherTeleporter->TeleportVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FVector TargetLocation = OtherTeleporter->TeleportTarget
		? OtherTeleporter->TeleportTarget->GetComponentLocation()
		: FVector::ZeroVector;
	FRotator TargetRotation = OtherTeleporter->TeleportTarget
		? OtherTeleporter->TeleportTarget->GetComponentRotation()
		: FRotator::ZeroRotator;
	OverlappingCharacter->SetActorLocationAndRotation(TargetLocation, TargetRotation);

	SetHighlighted_Implementation(bIsActive);
	UpdateTeleporterColour();
	if (OtherTeleporter) OtherTeleporter->UpdateTeleporterColour();
}

bool ATeleporter::IsCorrectItem_Implementation(const FInventorySlot& Slot) const
{
	return Slot.ItemAsset && Slot.ItemAsset->ItemType == RequiredItem;
}

void ATeleporter::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority()) return;
	if (!Interactor || !OtherTeleporter) return;

	// Always reset both teleporters to initial state
	bIsActive = true;
	OtherTeleporter->bIsActive = true;

	if (TeleportVolume)
		TeleportVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	if (OtherTeleporter->TeleportVolume)
		OtherTeleporter->TeleportVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	// Remove any outline/highlight
	SetHighlighted_Implementation(false);
	OtherTeleporter->SetHighlighted_Implementation(false);

	UpdateTeleporterColour();
	if (OtherTeleporter) OtherTeleporter->UpdateTeleporterColour();
}

void ATeleporter::SetHighlighted_Implementation(bool bHighlight)
{
	// Always allow turning OFF highlight; only allow turning on highlight when inactive
	if (!bHighlight || !bIsActive)
	{
		if (TeleporterMesh)
		{
			TeleporterMesh->SetRenderCustomDepth(bHighlight);
			TeleporterMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
		}
	}
}