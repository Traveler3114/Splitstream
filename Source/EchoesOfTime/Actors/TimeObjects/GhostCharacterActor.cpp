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
#include "Actors/RefPointActor.h"

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



bool AGhostCharacterActor::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
    // RealViewer is usually a PlayerController
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

    DrawDebugSphere(
        GetWorld(),
        GetActorLocation(),
        30.0f, 12, FColor::Green, false, -1.0f, 0, 2.0f
    );

    // Always update mesh/pose/material on ALL instances, not just server!
    ACharacter* CharacterToMirror = Cast<ACharacter>(GetOwner());
    if (!CharacterToMirror || !GhostMesh)
        return;

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
        GhostMesh->SetLeaderPoseComponent(SourceMesh, true, true);
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
        SetActorLocation(CharacterToMirror->GetActorLocation() + GhostOffset);
        SetActorRotation(CharacterToMirror->GetActorRotation());
    }
    // Do NOT set location/rotation on clients!
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

// IGhostRevealable implementation
void AGhostCharacterActor::SetGhostRevealed_Implementation(bool bRevealed)
{
    SetIsPastEchoAbilityActive(bRevealed);
}

void AGhostCharacterActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    // No need to replicate bIsPastEchoAbilityActive!
}