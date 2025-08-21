// Fill out your copyright notice in the Description page of Project Settings.


#include "KeycardScanner.h"

// Sets default values
AKeycardScanner::AKeycardScanner()
{
    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    Mesh->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AKeycardScanner::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AKeycardScanner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

