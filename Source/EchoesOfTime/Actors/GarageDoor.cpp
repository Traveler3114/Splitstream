// Fill out your copyright notice in the Description page of Project Settings.


#include "GarageDoor.h"

// Sets default values
AGarageDoor::AGarageDoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	GarageMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GarageMesh"));
	GarageMesh->SetupAttachment(RootComponent);
	GarageMesh->SetIsReplicated(true);
}

// Called when the game starts or when spawned
void AGarageDoor::BeginPlay()
{
	Super::BeginPlay();
}

void AGarageDoor::OnPuzzleCompleted_Implementation()
{
	// Logic to open the garage door
	UE_LOG(LogTemp, Log, TEXT("Garage door puzzle completed! Opening door."));
	// Example: Play an animation or change the position of GarageMesh to simulate opening
}

void AGarageDoor::OnPuzzleReset_Implementation()
{
	// Logic to close the garage door
	UE_LOG(LogTemp, Log, TEXT("Garage door puzzle reset! Closing door."));
	// Example: Play an animation or change the position of GarageMesh to simulate closing
}


