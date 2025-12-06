#include "GhostCharacterActor.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Interfaces/IGhostMirrorSource.h"
#include "GameFramework/PlayerController.h"
#include "DefaultPlayerState.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

AGhostCharacterActor::AGhostCharacterActor()
{
    PrimaryActorTick.bCanEverTick = true;

    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    GhostMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    GhostMesh->SetupAttachment(RootComponent);

    bReplicates = true;
    SetReplicateMovement(false); // We manually replicate target location/rotation

    Tags.AddUnique(TEXT("Ghost"));
}

bool AGhostCharacterActor::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
    const APlayerController* PC = Cast<APlayerController>(RealViewer);
    if (!PC)
        return false;

    const ADefaultPlayerState* PS = PC->GetPlayerState<ADefaultPlayerState>();
    if (PS && PS->GetTeamName().Equals(TEXT("Future"), ESearchCase::IgnoreCase))
    {
        return true;
    }
    return false;
}

void AGhostCharacterActor::BeginPlay()
{
    Super::BeginPlay();

    if (GhostMesh)
    {
        GhostMesh->SetVisibility(false, true);
        GhostMesh->SetHiddenInGame(false);
        GhostMesh->bOnlyOwnerSee = false;
        GhostMesh->bOwnerNoSee = false;
    }
}

void AGhostCharacterActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ACharacter* CharacterToMirror = Cast<ACharacter>(GetOwner());
    if (!CharacterToMirror || !GhostMesh)
        return;

    // Always interpolate smoothly - this is critical for visual quality
    float InterpSpeed = 5.f;
    SetActorLocation(FMath::VInterpTo(GetActorLocation(), GhostTargetLocation, DeltaTime, InterpSpeed));
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), GhostTargetRotation, DeltaTime, InterpSpeed));

    // Throttle expensive mesh/material updates to reduce CPU load
    TimeSinceLastUpdate += DeltaTime;
    if (TimeSinceLastUpdate < UpdateInterval)
    {
        return;
    }
    TimeSinceLastUpdate = 0.0f;

    // Update mesh/pose/material at reduced frequency
    USkeletalMeshComponent* SourceMesh = nullptr;
    if (CharacterToMirror->GetClass()->ImplementsInterface(UGhostMirrorSource::StaticClass()))
    {
        SourceMesh = IGhostMirrorSource::Execute_GetMirrorMesh(CharacterToMirror);
    }
    if (!SourceMesh)
    {
        SourceMesh = CharacterToMirror->FindComponentByClass<USkeletalMeshComponent>();
    }

    if (SourceMesh)
    {
        if (GhostMesh->GetSkeletalMeshAsset() != SourceMesh->GetSkeletalMeshAsset())
        {
            GhostMesh->SetSkeletalMeshAsset(SourceMesh->GetSkeletalMeshAsset());
        }
        SourceMesh->RefreshBoneTransforms();
        SourceMesh->UpdateComponentToWorld();
        GhostMesh->SetLeaderPoseComponent(SourceMesh, true, true);
    }
    else
    {
        // If no source mesh, reset leader pose
        GhostMesh->SetLeaderPoseComponent(nullptr, true, true);
    }

    if (GhostMaterial && GhostMesh->GetMaterial(0) != GhostMaterial)
    {
        const int32 NumMaterials = GhostMesh->GetNumMaterials();
        for (int32 i = 0; i < NumMaterials; ++i)
        {
            GhostMesh->SetMaterial(i, GhostMaterial);
        }
    }

    if (HasAuthority())
    {
        GhostTargetLocation = CharacterToMirror->GetActorLocation() + GhostOffset;
        GhostTargetRotation = CharacterToMirror->GetActorRotation();
    }
}

void AGhostCharacterActor::OnRep_GhostTargetLocation()
{
    // Snap instantly if very far away
    if (FVector::Dist(GetActorLocation(), GhostTargetLocation) > 500.f)
    {
        SetActorLocation(GhostTargetLocation);
    }
}

void AGhostCharacterActor::OnRep_GhostTargetRotation()
{
    // Snap instantly if very far away in rotation
    if (FMath::Abs((GetActorRotation() - GhostTargetRotation).GetNormalized().Yaw) > 45.f)
    {
        SetActorRotation(GhostTargetRotation);
    }
}

void AGhostCharacterActor::UpdateGhostVisibility()
{
    bool bShouldShow = false;
    bool bIsInCameraViewLocal = false;
    bool bPastEchoLocal = bIsPastEchoAbilityActive;

    ACharacter* CharacterToMirror = Cast<ACharacter>(GetOwner());
    FString OwnerName = CharacterToMirror ? CharacterToMirror->GetName() : TEXT("NONE");

    if (CharacterToMirror && CharacterToMirror->GetClass()->ImplementsInterface(UGhostMirrorSource::StaticClass()))
    {
        bIsInCameraViewLocal = IGhostMirrorSource::Execute_ShouldGhostBeVisible(CharacterToMirror);
        bShouldShow = bIsInCameraViewLocal && bPastEchoLocal;
    }

    if (GhostMesh)
    {
        GhostMesh->SetVisibility(bShouldShow, true);
        GhostMesh->SetHiddenInGame(false); // Always unhide for debug
    }
}

void AGhostCharacterActor::SetIsPastEchoAbilityActive(bool bActive)
{
    bIsPastEchoAbilityActive = bActive;
    UpdateGhostVisibility();
}

void AGhostCharacterActor::SetGhostRevealed_Implementation(bool bRevealed)
{
    SetIsPastEchoAbilityActive(bRevealed);
}

void AGhostCharacterActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AGhostCharacterActor, GhostTargetLocation);
    DOREPLIFETIME(AGhostCharacterActor, GhostTargetRotation);
    // No need to replicate bIsPastEchoAbilityActive!
}