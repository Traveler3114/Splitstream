// DroneSpawner.cpp
#include "DroneSpawner.h"
#include "Characters/DronePawn.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/PointActors/NavNode.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ActorComponents/SearchComponent.h"
#include "ActorComponents/InventoryComponent.h"
#include "UtilityLibrary.h"
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
	if (SearchComponent)
	{
		SearchComponent->bSearched = false;
	}
	if (bPausedFromSearchOrDestroy)
	{
		ResumeAllTimers();
	}
}

void ADroneSpawner::OnPuzzleCompleted_Implementation()
{
	if (SearchComponent) 
	{
		SearchComponent->bSearched = true;
		PauseAllTimers();
	}
}

void ADroneSpawner::Interact_Implementation(AActor* Interactor)
{
	if (SearchComponent)
	{
		SearchComponent->Interact(Interactor);
	}
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

	SetHighlighted_Implementation(false);

	PauseAllTimers();
}


void ADroneSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UUtilityLibrary::UnregisterRepairable(this, this);
    Super::EndPlay(EndPlayReason);
}

void ADroneSpawner::BeginPlay()
{
	Super::BeginPlay();

	UUtilityLibrary::RegisterRepairable(this, this);
	if (PlatformMesh)
		PlatformStartZ = PlatformMesh->GetRelativeLocation().Z;

	//TArray<AActor*> FoundDrones;
	//UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADronePawn::StaticClass(), FoundDrones);
	//for (AActor* Actor : FoundDrones)
	//{
	//	ADronePawn* Drone = Cast<ADronePawn>(Actor);
	//	if (Drone)
	//	{
	//		BindToDroneDeath(Drone);
	//		SpawnedDrones.Add(Drone);
	//	}
	//}

	for (ADronePawn* Drone : SpawnedDrones)
	{
		if (Drone)
		{
			BindToDroneDeath(Drone);
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
	// Queue up another spawn request
	PendingSpawnCount++;

	// If we aren't already spawning, start! Otherwise let the queue handle next
	if (!PendingDrone)
	{
		StartNextPendingSpawn();
	}
}

void ADroneSpawner::StartNextPendingSpawn()
{
    if (PendingSpawnCount <= 0) return;

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
            PendingDrone->SetRevealProgress(0.f); // Start fully hidden
        }
    }

    if (CountdownText)
    {
        CountdownText->SetText(FText::Format(NSLOCTEXT("DroneSpawner", "RespawnIn", "Spawning Drone in {0}"), FText::AsNumber((int32)RespawnTimeLeft)));
        CountdownText->SetVisibility(true);
    }

    RespawnTimeLeft = RespawnDelay;
    TimerAnimElapsed = 0.f;
    bIsPlatformReverse = false;
    
    // <-- Add this line for robust delta timing!
    LastPlatformAnimTime = GetWorld()->GetTimeSeconds();

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
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float DeltaTime = CurrentTime - LastPlatformAnimTime;
    LastPlatformAnimTime = CurrentTime;

    if (!bIsPlatformReverse)
    {
        TimerAnimElapsed += DeltaTime;
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

            // ---- Reveal mesh gradually ----
            if (PendingDrone)
            {
                PendingDrone->SetRevealProgress(S); // S goes from 0 to 1 during platform up
            }
        }
    }
    else
    {
        float DownInterval = DeltaTime * 3;
        PlatformDownAnimElapsed += DownInterval;
        float DownDuration = RespawnDelay / 3; // finish faster
        float Alpha = FMath::Clamp(PlatformDownAnimElapsed / DownDuration, 0.f, 1.f);
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

	// One spawn done, decrement
	PendingSpawnCount--;
	if (PendingSpawnCount > 0)
	{
		// Spawn next, if there's another pending
		StartNextPendingSpawn();
	}
}

void ADroneSpawner::ActivatePendingDrone()
{
	if (PendingDrone)
	{
		PendingDrone->SetRevealProgress(1.f);
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
