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

	TeleportVolume->OnComponentBeginOverlap.AddDynamic(this, &ATeleporter::OnTeleportVolumeBeginOverlap);
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
	if (!OtherTeleporter) return;
	if (!OtherActor || !HasAuthority()) return;
	if (!bIsActive || !OtherTeleporter->bIsActive) return;

	// Deactivate both BEFORE teleporting to avoid recursion
	bIsActive = false;
	OtherTeleporter->bIsActive = false;

	TeleportVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OtherTeleporter->TeleportVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ACharacter* OverlappingCharacter = Cast<ACharacter>(OtherActor);
	if (OverlappingCharacter)
	{
		FVector TargetLocation = OtherTeleporter->TeleportTarget->GetComponentLocation();
		FRotator TargetRotation = OtherTeleporter->TeleportTarget->GetComponentRotation();
		OverlappingCharacter->SetActorLocationAndRotation(TargetLocation, TargetRotation);
	}
}

bool ATeleporter::IsCorrectItem_Implementation(UItemBase* Item) const
{
	// This is the ONLY place that checks if the item is the correct keycard.
	return Item && Item->ItemType == RequiredItem;
}
void ATeleporter::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority()) return;
	if (!Interactor || !OtherTeleporter)
		return;
	if (bIsActive)  // Only allow interaction when inactive
		return;

	// Reactivate this teleporter and the linked teleporter
	bIsActive = true;
	OtherTeleporter->bIsActive = true;

	// Re-enable the collision on both volumes
	if (TeleportVolume)
		TeleportVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	if (OtherTeleporter->TeleportVolume)
		OtherTeleporter->TeleportVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void ATeleporter::SetHighlighted_Implementation(bool bHighlight)
{
	if (bIsActive)  // Only allow interaction when inactive
		return;
	if (TeleporterMesh)
	{
		TeleporterMesh->SetRenderCustomDepth(bHighlight);
		TeleporterMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
	}
}