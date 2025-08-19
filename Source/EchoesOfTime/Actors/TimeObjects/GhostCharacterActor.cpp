// Copyright

#include "GhostCharacterActor.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Characters/GuardCharacter.h"
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

    if (!CharacterToMirror || !CharacterToMirror->GetMesh() || !GhostMesh)
    {
        return;
    }

    // Set the mesh once if not already set
    if (GhostMesh->GetSkeletalMeshAsset() != CharacterToMirror->GetMesh()->GetSkeletalMeshAsset())
    {
        GhostMesh->SetSkeletalMeshAsset(CharacterToMirror->GetMesh()->GetSkeletalMeshAsset());
    }

    // Mirror animation pose
    GhostMesh->SetLeaderPoseComponent(CharacterToMirror->GetMesh(), true, true);

    // Apply ghost material once (optional optimization)
    if (GhostMaterial && GhostMesh->GetMaterial(0) != GhostMaterial)
    {
        const int32 NumMaterials = GhostMesh->GetNumMaterials();
        for (int32 i = 0; i < NumMaterials; ++i)
        {
            GhostMesh->SetMaterial(i, GhostMaterial);
        }
    }

    // Sync location and rotation
    SetActorLocation(CharacterToMirror->GetActorLocation() + GhostOffset);
    SetActorRotation(CharacterToMirror->GetActorRotation());
}

void AGhostCharacterActor::UpdateGhostVisibility()
{
    bool bInCameraView = false;
    if (CharacterToMirror)
    {
        // Cast just in case (if CharacterToMirror is always a GuardCharacter, you can static_cast)
        const AGuardCharacter* Guard = Cast<AGuardCharacter>(CharacterToMirror);
        if (Guard)
        {
            bInCameraView = Guard->bIsInCameraView;
        }
    }

    bool bShouldShow = bInCameraView && bIsPastEchoAbilityActive;
    if (GhostMesh)
    {
        GhostMesh->SetVisibility(bShouldShow, true);
    }
}

void AGhostCharacterActor::SetIsPastEchoAbilityActive(bool bActive)
{
    bIsPastEchoAbilityActive = bActive;
    UpdateGhostVisibility();
}

void AGhostCharacterActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGhostCharacterActor, CharacterToMirror);
	//DOREPLIFETIME(AGhostCharacterActor, GhostMaterial);
}