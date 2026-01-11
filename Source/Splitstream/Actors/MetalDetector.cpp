// Fill out your copyright notice in the Description page of Project Settings.

#include "MetalDetector.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Characters/DefaultCharacter.h"
#include "GameStates/DefaultGameState.h"

AMetalDetector::AMetalDetector()
{
	PrimaryActorTick.bCanEverTick = false;

	// Mesh Setup
	DetectorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DetectorMesh"));
	RootComponent = DetectorMesh;

	// Collision Setup
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	TriggerBox->SetBoxExtent(FVector(100.0f, 50.0f, 200.0f)); // Example size
	bReplicates = true;
}

void AMetalDetector::BeginPlay()
{
	Super::BeginPlay();
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AMetalDetector::OnPlayerOverlap);
}

void AMetalDetector::OnPlayerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	// Run on server only to avoid double triggers
	if (!HasAuthority())
	{
		return;
	}

	if (OtherActor && OtherActor->IsA(ADefaultCharacter::StaticClass()) && bEnabled && OtherActor != this)
	{
		if (ADefaultGameState* GS = Cast<ADefaultGameState>(GetWorld()->GetGameState()))
		{
			// start alarm instead of immediate restart
			GS->StartAlarm();
		}
	}
}

void AMetalDetector::OnPuzzleCompleted_Implementation()
{
    // This will be called by LeverManager when its puzzle is completed
    bEnabled = false; // Or whatever logic
    OnRep_Enabled();  // Notify clients immediately
}

void AMetalDetector::OnRep_Enabled()
{
	// This is called on clients when bEnabled changes.
	// You can play effects or update visuals based on state.
}

void AMetalDetector::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMetalDetector, bEnabled);
}