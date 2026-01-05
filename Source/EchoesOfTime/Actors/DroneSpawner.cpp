#include "DroneSpawner.h"
#include "Characters/DronePawn.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/PointActors/NavNode.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ActorComponents/SearchComponent.h"
#include "ActorComponents/InventoryComponent.h"
#include "TimerManager.h"

ADroneSpawner::ADroneSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

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

void ADroneSpawner::RequestRepair_Implementation(AActor* RepairInstigator)
{
	// Un-destroy/unsearch and resume timers
	if (SearchComponent)
	{
		SearchComponent->bSearched = false;
	}
	if (bPausedFromSearchOrDestroy)
	{
		ResumeAllTimers();
	}
}

void ADroneSpawner::Interact_Implementation(AActor* Interactor)
{
	if (SearchComponent)
	{
		SearchComponent->Interact(Interactor);
	}
	// If bSearched became true, PAUSE timers (don't reset)
	if (SearchComponent && SearchComponent->bSearched)
	{
		PauseAllTimers();
	}
}

void ADroneSpawner::CancelInteract_Implementation(AActor* Interactor)
{
	if (SearchComponent)
	{
		SearchComponent->CancelInteract(Interactor);
	}
	// If bSearched became true, PAUSE timers
	if (SearchComponent && SearchComponent->bSearched)
	{
		PauseAllTimers();
	}
}

void ADroneSpawner::SetHighlighted_Implementation(bool bHighlight)
{
	if (PrinterMesh && SearchComponent)
	{
		if (SearchComponent->bSearched)
		{
			PrinterMesh->SetRenderCustomDepth(false);
			PrinterMesh->CustomDepthStencilValue = 0;
			PauseAllTimers();
		}
		else
		{
			PrinterMesh->SetRenderCustomDepth(bHighlight);
			PrinterMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
			ResumeAllTimers();
		}
	}
}

void ADroneSpawner::OnSearchComplete()
{
	if (!RewardItem) return;
	if (!HasAuthority()) return;

	if (!SearchComponent) return;

	AActor* LastInteractor = SearchComponent->LastInteractor.Get();
	if (!LastInteractor) return;

	UInventoryComponent* Inventory = LastInteractor->FindComponentByClass<UInventoryComponent>();
	if (!Inventory) return;

	FGuid NewInstanceID = FGuid::NewGuid();
	bool bAdded = Inventory->AddItem(RewardItem, NewInstanceID);

	OnRepairRequested.Broadcast(this);

	// Set highlight off and pause timers if searched
	SetHighlighted_Implementation(false);

	PauseAllTimers();
}

void ADroneSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (PlatformMesh)
		PlatformStartZ = PlatformMesh->GetRelativeLocation().Z;

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

	RespawnTimeLeft = RespawnDelay;
	TimerAnimElapsed = 0.f;
	bIsPlatformReverse = false;

	if (PendingDrone)
	{
		PendingDrone->Destroy();
		PendingDrone = nullptr;
	}

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

	if (CountdownText)
	{
		CountdownText->SetText(FText::Format(NSLOCTEXT("DroneSpawner", "RespawnIn", "Spawning Drone in {0}"), FText::AsNumber((int32)RespawnTimeLeft)));
		CountdownText->SetVisibility(true);
	}

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
			CountdownText->SetText(FText::AsNumber((int32)RespawnTimeLeft));
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

		if (PlatformMesh)
		{
			FVector RelLoc = PlatformMesh->GetRelativeLocation();
			RelLoc.Z = PlatformStartZ + PlatformRiseOffset * S;
			PlatformMesh->SetRelativeLocation(RelLoc);
		}

		if (PendingDrone && PlatformMesh)
		{
			FVector PlatLoc = PlatformMesh->GetComponentLocation() + DroneSpawnOffset;
			PendingDrone->SetActorLocation(PlatLoc);
		}
	}
	else
	{
		PlatformDownAnimElapsed += TickInterval;
		float Alpha = FMath::Clamp(PlatformDownAnimElapsed / RespawnDelay, 0.f, 1.f);
		float S = 1.f - FMath::SmoothStep(0.f, 1.f, Alpha);

		if (PlatformMesh)
		{
			FVector RelLoc = PlatformMesh->GetRelativeLocation();
			RelLoc.Z = PlatformStartZ + PlatformRiseOffset * S;
			PlatformMesh->SetRelativeLocation(RelLoc);
		}

		if (Alpha >= 1.f)
		{
			ResetPlatform();
			GetWorldTimerManager().ClearTimer(PlatformAnimTimerHandle);
		}
	}
}

void ADroneSpawner::OnRespawnTimerFinished()
{
	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
	GetWorldTimerManager().ClearTimer(TextUpdateTimerHandle);

	if (CountdownText)
		CountdownText->SetVisibility(false);

	RespawnTimeLeft = 0.0f;

	ActivatePendingDrone();
}

void ADroneSpawner::ActivatePendingDrone()
{
	if (PendingDrone)
	{
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
	if (PlatformMesh)
	{
		FVector RelLoc = PlatformMesh->GetRelativeLocation();
		RelLoc.Z = PlatformStartZ;
		PlatformMesh->SetRelativeLocation(RelLoc);
	}
	bIsPlatformReverse = false;
	TimerAnimElapsed = 0.f;
	PlatformDownAnimElapsed = 0.f;
}

// ----------- PAUSE/RESUME LOGIC ----------------------

void ADroneSpawner::PauseAllTimers()
{
	if (bPausedFromSearchOrDestroy) return; // Already paused

	UWorld* World = GetWorld();
	if (!World) return;

	// Store all remaining/progress state
	bPausedFromSearchOrDestroy = true;
	PausedRespawnTimeLeft = RespawnTimeLeft;
	PausedTimerAnimElapsed = TimerAnimElapsed;
	PausedPlatformDownAnimElapsed = PlatformDownAnimElapsed;

	World->GetTimerManager().PauseTimer(RespawnTimerHandle);
	World->GetTimerManager().PauseTimer(TextUpdateTimerHandle);
	World->GetTimerManager().PauseTimer(PlatformAnimTimerHandle);
}

void ADroneSpawner::ResumeAllTimers()
{
	if (!bPausedFromSearchOrDestroy) return; // Not paused

	UWorld* World = GetWorld();
	if (!World) return;

	// Resume all timers from their paused state
	bPausedFromSearchOrDestroy = false;

	World->GetTimerManager().UnPauseTimer(RespawnTimerHandle);
	World->GetTimerManager().UnPauseTimer(TextUpdateTimerHandle);
	World->GetTimerManager().UnPauseTimer(PlatformAnimTimerHandle);

	// Restore the time values for internal logic
	RespawnTimeLeft = PausedRespawnTimeLeft;
	TimerAnimElapsed = PausedTimerAnimElapsed;
	PlatformDownAnimElapsed = PausedPlatformDownAnimElapsed;
}