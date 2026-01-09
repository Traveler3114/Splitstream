// Fill out your copyright notice in the Description page of Project Settings.

#include "DroneSpawnerDisabler.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/DroneSpawner.h"
#include "Net/UnrealNetwork.h"
#include "Interfaces/IPuzzleCompletionReceiver.h"

// Sets default values
ADroneSpawnerDisabler::ADroneSpawnerDisabler()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SearchComponent = CreateDefaultSubobject<USearchComponent>(TEXT("SearchComponent"));
	SearchComponent->SetIsReplicated(true);

	DeviceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DeviceMesh"));
	DeviceMesh->SetIsReplicated(true);
}


void ADroneSpawnerDisabler::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADroneSpawnerDisabler, bVisualOnly);
}

void ADroneSpawnerDisabler::BeginPlay()
{
	Super::BeginPlay();
	SearchComponent->OnSearchComplete.AddDynamic(this, &ADroneSpawnerDisabler::OnSearchComplete);
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADroneSpawner::StaticClass(), CompletionTargets);
	if (HasAuthority())
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

			bVisualOnly = true;
		}
		// Optionally add else clause to assign null or fallback mesh
	}
}

void ADroneSpawnerDisabler::OnSearchComplete()
{
	if (bVisualOnly)
		return;
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
	if (bVisualOnly)
		return;
	if (SearchComponent)
		SearchComponent->Interact(Interactor);
}

void ADroneSpawnerDisabler::CancelInteract_Implementation(AActor* Interactor)
{
	if (bVisualOnly)
		return;
	if (SearchComponent)
		SearchComponent->CancelInteract(Interactor);
}

void ADroneSpawnerDisabler::SetHighlighted_Implementation(bool bHighlighted)
{
	if (bVisualOnly)
		return;
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