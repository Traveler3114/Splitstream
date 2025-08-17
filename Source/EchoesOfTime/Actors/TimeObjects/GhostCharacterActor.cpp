// Fill out your copyright notice in the Description page of Project Settings.


#include "GhostCharacterActor.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"

// Sets default values
AGhostCharacterActor::AGhostCharacterActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	// Create skeletal mesh component
	GhostMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	GhostMesh->SetupAttachment(RootComponent);

	bReplicates = true;
	SetReplicateMovement(true);

}

// Called when the game starts or when spawned
void AGhostCharacterActor::BeginPlay()
{
	Super::BeginPlay();
	GhostMesh->bOnlyOwnerSee = true; // Only the owner can see this mesh
}

// Called every frame
void AGhostCharacterActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Draw a debug sphere at the actor's location
	DrawDebugSphere(
		GetWorld(),
		GetActorLocation(),
		30.0f,           // Radius
		12,              // Segments
		FColor::Green,   // Color
		false,           // Persistent lines
		-1.0f,           // Life time
		0,               // Depth priority
		2.0f             // Thickness
	);



	// Debug: Mesh assignment and error reporting
	if (CharacterToMirror && CharacterToMirror->GetMesh() && GhostMesh)
	{
		USkeletalMesh* MeshAsset = CharacterToMirror->GetMesh()->GetSkeletalMeshAsset();
		if (MeshAsset)
		{
			GhostMesh->SetSkeletalMesh(MeshAsset);
			GhostMesh->SetLeaderPoseComponent(CharacterToMirror->GetMesh(), true, true);

		}

		// Material assignment
		if (GhostMaterial)
		{
			const int32 Num = GhostMesh->GetNumMaterials();
			for (int32 i = 0; i < Num; ++i)
			{
				GhostMesh->SetMaterial(i, GhostMaterial);
			}
		}
	}

	// Sync location and rotation
	if (CharacterToMirror)
	{
		SetActorLocation(CharacterToMirror->GetActorLocation() + FVector(0, -4910, -80));
		SetActorRotation(CharacterToMirror->GetActorRotation());
	}
}

void AGhostCharacterActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGhostCharacterActor, CharacterToMirror);
	DOREPLIFETIME(AGhostCharacterActor, GhostMaterial);
}