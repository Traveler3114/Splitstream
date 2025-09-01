// Fill out your copyright notice in the Description page of Project Settings.


#include "KeypadScanner.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AKeypadScanner::AKeypadScanner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    // Create static mesh component and attach to root
    KeypadScannerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KeypadScannerMesh"));
    KeypadScannerMesh->SetupAttachment(DefaultSceneRoot);
}

// Called when the game starts or when spawned
void AKeypadScanner::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AKeypadScanner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

