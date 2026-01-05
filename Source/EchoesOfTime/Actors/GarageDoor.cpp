// Fill out your copyright notice in the Description page of Project Settings.


#include "GarageDoor.h"

// Sets default values
AGarageDoor::AGarageDoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	GarageBase = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GarageBase"));
	GarageBase->SetupAttachment(RootComponent);

	GarageDoor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GarageDoor"));
	GarageDoor->SetupAttachment(GarageBase);
	GarageDoor->SetIsReplicated(true);
}

// Called when the game starts or when spawned
void AGarageDoor::BeginPlay()
{
	Super::BeginPlay();
}

void AGarageDoor::OnPuzzleCompleted_Implementation()
{
	OpenDoor();
}

void AGarageDoor::OnPuzzleReset_Implementation()
{
	CloseDoor();
}


