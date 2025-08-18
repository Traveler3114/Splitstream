// Copyright

#include "GhostCharacterActor.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogGhostCharacter, Log, All);

// Sets default values
AGhostCharacterActor::AGhostCharacterActor()
{
	PrimaryActorTick.bCanEverTick = true;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	// Create skeletal mesh component
	GhostMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	GhostMesh->SetupAttachment(RootComponent);

	// Replicate actor and movement for syncing
	bReplicates = true;
	SetReplicateMovement(true);

	// Tag for cue lookup
	Tags.AddUnique(TEXT("Ghost"));
}

// Called when the game starts or when spawned
void AGhostCharacterActor::BeginPlay()
{
	Super::BeginPlay();

	// Start hidden for all clients. We'll selectively show it on individual clients.
	if (GhostMesh)
	{
		GhostMesh->SetVisibility(false, true);
		GhostMesh->bOnlyOwnerSee = false; // ensure we are not gating by single owner
	}

}


// Called every frame
void AGhostCharacterActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

	// Sync mesh and pose from mirrored character (if any)
	if (CharacterToMirror && CharacterToMirror->GetMesh() && GhostMesh)
	{
		if (USkeletalMesh* MeshAsset = CharacterToMirror->GetMesh()->GetSkeletalMeshAsset())
		{
			GhostMesh->SetSkeletalMesh(MeshAsset);
			GhostMesh->SetLeaderPoseComponent(CharacterToMirror->GetMesh(), true, true);
		}

		// Apply ghost material locally
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
	//DOREPLIFETIME(AGhostCharacterActor, GhostMaterial);
}