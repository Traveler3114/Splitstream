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

AGhostCharacterActor::AGhostCharacterActor()
{
    PrimaryActorTick.bCanEverTick = true;

    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    GhostMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    GhostMesh->SetupAttachment(RootComponent);

    bReplicates = true;
    SetReplicateMovement(true);

    Tags.AddUnique(TEXT("Ghost"));
}

void AGhostCharacterActor::BeginPlay()
{
    Super::BeginPlay();

    if (GhostMesh)
    {
        GhostMesh->SetVisibility(false, true);
        GhostMesh->bOnlyOwnerSee = false;
    }
}

void AGhostCharacterActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    DrawDebugSphere(
        GetWorld(),
        GetActorLocation(),
        30.0f, 12, FColor::Green, false, -1.0f, 0, 2.0f
    );

    if (!CharacterToMirror || !CharacterToMirror->GetMesh() || !GhostMesh)
    {
        return;
    }

    if (GhostMesh->GetSkeletalMeshAsset() != CharacterToMirror->GetMesh()->GetSkeletalMeshAsset())
    {
        GhostMesh->SetSkeletalMeshAsset(CharacterToMirror->GetMesh()->GetSkeletalMeshAsset());
    }

    GhostMesh->SetLeaderPoseComponent(CharacterToMirror->GetMesh(), true, true);

    if (GhostMaterial && GhostMesh->GetMaterial(0) != GhostMaterial)
    {
        const int32 NumMaterials = GhostMesh->GetNumMaterials();
        for (int32 i = 0; i < NumMaterials; ++i)
        {
            GhostMesh->SetMaterial(i, GhostMaterial);
        }
    }

    SetActorLocation(CharacterToMirror->GetActorLocation() + GhostOffset);
    SetActorRotation(CharacterToMirror->GetActorRotation());
}

void AGhostCharacterActor::UpdateGhostVisibility()
{
    bool bInCameraView = false;
    if (CharacterToMirror)
    {
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

// IGhostRevealable implementation
void AGhostCharacterActor::SetGhostRevealed_Implementation(bool bRevealed)
{
    SetIsPastEchoAbilityActive(bRevealed);
}

void AGhostCharacterActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AGhostCharacterActor, CharacterToMirror);
}