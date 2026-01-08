// Fill out your copyright notice in the Description page of Project Settings.

#include "DroneSpawnerDisabler.h"
#include "Interfaces/IPuzzleCompletionReceiver.h"

// Sets default values
ADroneSpawnerDisabler::ADroneSpawnerDisabler()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SearchComponent = CreateDefaultSubobject<USearchComponent>(TEXT("SearchComponent"));
	SearchComponent->SetIsReplicated(true);
}

void ADroneSpawnerDisabler::BeginPlay()
{
	Super::BeginPlay();
	SearchComponent->OnSearchComplete.AddDynamic(this, &ADroneSpawnerDisabler::OnSearchComplete);

	SetMesh();
}

void ADroneSpawnerDisabler::SetMesh()
{
	if (DeviceMesh)
	{
		if (TimelineEra == ETimelineEra::Past && PastMesh)
		{
			DeviceMesh->SetStaticMesh(PastMesh);
		}
		else if (TimelineEra == ETimelineEra::Future && FutureMesh)
		{
			DeviceMesh->SetStaticMesh(FutureMesh);
		}
		// Optionally add else clause to assign null or fallback mesh
	}
}

void ADroneSpawnerDisabler::OnSearchComplete()
{
	for (AActor* Target : CompletionTargets)
	{
		if (Target && Target->GetClass()->ImplementsInterface(UPuzzleCompletionReceiver::StaticClass()))
		{
			IPuzzleCompletionReceiver::Execute_OnPuzzleCompleted(Target);
		}
	}
	SetHighlighted_Implementation(false);
}

void ADroneSpawnerDisabler::Interact_Implementation(AActor* Interactor)
{
	if (SearchComponent)
		SearchComponent->Interact(Interactor);
}

void ADroneSpawnerDisabler::CancelInteract_Implementation(AActor* Interactor)
{
	if (SearchComponent)
		SearchComponent->CancelInteract(Interactor);
}

void ADroneSpawnerDisabler::SetHighlighted_Implementation(bool bHighlighted)
{
	if (DeviceMesh && SearchComponent)
	{
		if (SearchComponent->bSearched)
		{
			DeviceMesh->SetRenderCustomDepth(false);
			DeviceMesh->CustomDepthStencilValue = 0;
			return;
		}
		DeviceMesh->SetRenderCustomDepth(bHighlighted);
		DeviceMesh->CustomDepthStencilValue = bHighlighted ? 1 : 0;
	}
}