// DroneSpawner.cpp
#include "DroneSpawner.h"
#include "Characters/DronePawn.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/PointActors/NavNode.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ActorComponents/SearchComponent.h"
#include "ActorComponents/InventoryComponent.h"
#include "TimerManager.h"

// Sets default values
ADroneSpawner::ADroneSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	// Make PrinterMesh the root
	PrinterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrinterMesh"));
	RootComponent = PrinterMesh;

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	PlatformMesh->SetupAttachment(PrinterMesh);

	SearchComponent = CreateDefaultSubobject<USearchComponent>(TEXT("SearchComponent"));
	SearchComponent->SetIsReplicated(true);

	CountdownText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("CountdownText"));
	CountdownText->SetupAttachment(PrinterMesh);
	CountdownText->SetText(FText::GetEmpty());
	CountdownText->SetHorizontalAlignment(EHTA_Center);
	CountdownText->SetVisibility(false);
}

void ADroneSpawner::Interact_Implementation(AActor* Interactor)
{
	if(SearchComponent)
	{
		SearchComponent->Interact(Interactor);
	}
}

void ADroneSpawner::CancelInteract_Implementation(AActor* Interactor)
{
	if (SearchComponent)
	{
		SearchComponent->CancelInteract(Interactor);
	}
}

void ADroneSpawner::SetHighlighted_Implementation(bool bHighlight)
{
	if (PrinterMesh && !SearchComponent->bSearched)
	{
		PrinterMesh->SetRenderCustomDepth(bHighlight);
		PrinterMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
	}
}

void ADroneSpawner::OnSearchComplete() 
{
	if (!RewardItem) return;

	if (!HasAuthority()) return; // Ensure only server gives the item

	// Null check for SearchComponent
	if (!SearchComponent) {
		return;
	}

	// Null check for LastInteractor
	AActor* LastInteractor = SearchComponent->LastInteractor.Get();
	if (!LastInteractor) {
		return;
	}

	UInventoryComponent* Inventory = LastInteractor->FindComponentByClass<UInventoryComponent>();
	if (!Inventory) {
		return;
	}

	FGuid NewInstanceID = FGuid::NewGuid();
	bool bAdded = Inventory->AddItem(RewardItem, NewInstanceID);
}

void ADroneSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (PlatformMesh)
		PlatformStartZ = PlatformMesh->GetRelativeLocation().Z;

	// Optionally, catch already existing drones at runtime
	TArray<AActor*> FoundDrones;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADronePawn::StaticClass(), FoundDrones);
	for (AActor* Actor : FoundDrones)
	{
		ADronePawn* Drone = Cast<ADronePawn>(Actor);
		if (Drone)
		{
			BindToDroneDeath(Drone);
			SpawnedDrones.Add(Drone);
		}
	}

	if (SearchComponent)
	{
		SearchComponent->OnSearchComplete.AddDynamic(this, &ADroneSpawner::OnSearchComplete);
	}
}

void ADroneSpawner::BindToDroneDeath(ADronePawn* Drone)
{
	if (Drone)
	{
		Drone->OnDroneDeath.RemoveDynamic(this, &ADroneSpawner::HandleDroneDeath);
		Drone->OnDroneDeath.AddDynamic(this, &ADroneSpawner::HandleDroneDeath);
	}
}

void ADroneSpawner::HandleDroneDeath(ADronePawn* DeadDrone)
{
	SpawnedDrones.Remove(DeadDrone);

	// Start respawn timer
	RespawnTimeLeft = RespawnDelay;
	TimerAnimElapsed = 0.f;
	bIsPlatformReverse = false; // Rising up

	// Only one pending drone at once
	if (PendingDrone)
	{
		PendingDrone->Destroy();
		PendingDrone = nullptr;
	}

	// Spawn PendingDrone as INACTIVE, on platform
	if (DroneClass && PlatformMesh)
	{
		FVector SpawnLoc = PlatformMesh->GetComponentLocation() + DroneSpawnOffset;
		FRotator SpawnRot = GetActorRotation();
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		PendingDrone = GetWorld()->SpawnActor<ADronePawn>(DroneClass, SpawnLoc, SpawnRot, Params);
		PendingDrone->TimelineEra = TimelineEra;

		if (PendingDrone)
		{
			PendingDrone->DeactivateDrone();
		}
	}

	// Show countdown
	if (CountdownText)
	{
		CountdownText->SetText(FText::Format(NSLOCTEXT("DroneSpawner", "RespawnIn", "Spawning Drone in {0}"), FText::AsNumber((int32)RespawnTimeLeft)));
		CountdownText->SetVisibility(true);
	}

	// Start timers: platform up anim, text, countdown
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ADroneSpawner::OnRespawnTimerFinished, RespawnDelay, false);
	GetWorldTimerManager().SetTimer(TextUpdateTimerHandle, this, &ADroneSpawner::UpdateCountdownText, 1.0f, true);
	GetWorldTimerManager().SetTimer(PlatformAnimTimerHandle, this, &ADroneSpawner::TickPlatformAnim, 0.02f, true);
}

void ADroneSpawner::UpdateCountdownText()
{
	RespawnTimeLeft -= 1.0f;
	if (RespawnTimeLeft > 0)
	{
		if (CountdownText)
		{
			CountdownText->SetText(FText::AsNumber((int32)RespawnTimeLeft));
		}
	}
	else
	{
		GetWorldTimerManager().ClearTimer(TextUpdateTimerHandle);
	}
}

void ADroneSpawner::TickPlatformAnim()
{
	const float TickInterval = 0.02f;
	if (!bIsPlatformReverse)
	{
		TimerAnimElapsed += TickInterval;
		float Alpha = FMath::Clamp(TimerAnimElapsed / RespawnDelay, 0.f, 1.f);
		float S = FMath::SmoothStep(0.f, 1.f, Alpha);

		// Move platform up
		if (PlatformMesh)
		{
			FVector RelLoc = PlatformMesh->GetRelativeLocation();
			RelLoc.Z = PlatformStartZ + PlatformRiseOffset * S;
			PlatformMesh->SetRelativeLocation(RelLoc);
		}

		// Move pending drone with platform
		if (PendingDrone && PlatformMesh)
		{
			FVector PlatLoc = PlatformMesh->GetComponentLocation() + DroneSpawnOffset;
			PendingDrone->SetActorLocation(PlatLoc);
		}

		if (Alpha >= 1.f)
		{
			// Don't clear timer here; let it keep ticking until reverse is done.
			// Timer will be reversed after ActivatePendingDrone is called!
		}
	}
	else // Going down
	{
		PlatformDownAnimElapsed += TickInterval;
		float Alpha = FMath::Clamp(PlatformDownAnimElapsed / RespawnDelay, 0.f, 1.f);
		float S = 1.f - FMath::SmoothStep(0.f, 1.f, Alpha); // invert

		if (PlatformMesh)
		{
			FVector RelLoc = PlatformMesh->GetRelativeLocation();
			RelLoc.Z = PlatformStartZ + PlatformRiseOffset * S;
			PlatformMesh->SetRelativeLocation(RelLoc);
		}

		if (Alpha >= 1.f)
		{
			// Fully down, stop anim and snap to original just to be exact
			ResetPlatform();
			GetWorldTimerManager().ClearTimer(PlatformAnimTimerHandle);
		}
	}
}

void ADroneSpawner::OnRespawnTimerFinished()
{
	// Stop platform going up (it will reverse in ActivatePendingDrone)
	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
	GetWorldTimerManager().ClearTimer(TextUpdateTimerHandle);

	// Hide the countdown text
	if (CountdownText)
		CountdownText->SetVisibility(false);

	RespawnTimeLeft = 0.0f;

	// Activate the drone and start moving platform down
	ActivatePendingDrone();
}

void ADroneSpawner::ActivatePendingDrone()
{
	// Activate real spawned drone.
	if (PendingDrone)
	{

		// Usual navigation assignment
		TArray<AActor*> FoundNodes;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANavNode::StaticClass(), FoundNodes);

		TArray<ANavNode*> MatchingNodes;
		for (AActor* Actor : FoundNodes)
		{
			ANavNode* Node = Cast<ANavNode>(Actor);
			if (Node && Node->TimelineEra == PendingDrone->TimelineEra && Node->NodeType == ENavNodeType::Sky)
			{
				MatchingNodes.Add(Node);
			}
		}

		BindToDroneDeath(PendingDrone);
		SpawnedDrones.Add(PendingDrone);

		if (MatchingNodes.Num() > 0)
		{
			ANavNode* SpawnNode = MatchingNodes[FMath::RandHelper(MatchingNodes.Num())];
			PendingDrone->ActivateDrone(SpawnNode);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No NavNodes found for TimelineEra and NodeType that matches the drone! Drone will remain at spawn location."));
		}

		PendingDrone = nullptr;
	}
	bIsPlatformReverse = true;
	PlatformDownAnimElapsed = 0.f;
	GetWorldTimerManager().SetTimer(PlatformAnimTimerHandle, this, &ADroneSpawner::TickPlatformAnim, 0.02f, true);
}


void ADroneSpawner::ResetPlatform()
{
	// Reset platform Z
	if (PlatformMesh)
	{
		FVector RelLoc = PlatformMesh->GetRelativeLocation();
		RelLoc.Z = PlatformStartZ;
		PlatformMesh->SetRelativeLocation(RelLoc);
	}
	// Ready for the next cycle
	bIsPlatformReverse = false;
	TimerAnimElapsed = 0.f;
	PlatformDownAnimElapsed = 0.f;
}