#include "GhostCharacterActor.h"
#include "Components/SceneComponent.h"
#include "GameplayTagContainer.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Interfaces/IGhostMirrorSource.h"
#include "GameFramework/PlayerController.h"
#include "DefaultPlayerState.h"
#include "Engine/Engine.h"

AGhostCharacterActor::AGhostCharacterActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

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
    if (!PC) return false;

    const ADefaultPlayerState* PS = PC->GetPlayerState<ADefaultPlayerState>();
    FGameplayTag FutureTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Future"));
    if (PS && PS->GetTeamTag() == FutureTag)
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

    ACharacter* CharacterToMirror = Cast<ACharacter>(GetOwner());
    if (CharacterToMirror)
    {
        if (CharacterToMirror->GetClass()->ImplementsInterface(UGhostMirrorSource::StaticClass()))
        {
            CachedSourceMesh = IGhostMirrorSource::Execute_GetMirrorMesh(CharacterToMirror);
        }
        if (!CachedSourceMesh)
        {
            CachedSourceMesh = CharacterToMirror->FindComponentByClass<USkeletalMeshComponent>();
        }
    }
}

void AGhostCharacterActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!GhostMesh)
        return;

    if (!CachedSourceMesh)
    {
        ACharacter* CharacterToMirror = Cast<ACharacter>(GetOwner());
        if (CharacterToMirror)
        {
            if (CharacterToMirror->GetClass()->ImplementsInterface(UGhostMirrorSource::StaticClass()))
            {
                CachedSourceMesh = IGhostMirrorSource::Execute_GetMirrorMesh(CharacterToMirror);
            }
            if (!CachedSourceMesh)
            {
                CachedSourceMesh = CharacterToMirror->FindComponentByClass<USkeletalMeshComponent>();
            }
        }
    }

    if (CachedSourceMesh)
    {
        if (GhostMesh->GetSkeletalMeshAsset() != CachedSourceMesh->GetSkeletalMeshAsset())
        {
            GhostMesh->SetSkeletalMeshAsset(CachedSourceMesh->GetSkeletalMeshAsset());
        }
        CachedSourceMesh->RefreshBoneTransforms();
        CachedSourceMesh->UpdateComponentToWorld();
        GhostMesh->SetLeaderPoseComponent(CachedSourceMesh, true, true);
    }
    else
    {
        GhostMesh->SetLeaderPoseComponent(nullptr, true, true);
    }

    if (!bMaterialApplied && GhostMaterial && GhostMesh->GetMaterial(0) != GhostMaterial)
    {
        const int32 NumMaterials = GhostMesh->GetNumMaterials();
        for (int32 i = 0; i < NumMaterials; ++i)
        {
            GhostMesh->SetMaterial(i, GhostMaterial);
        }
        bMaterialApplied = true;
    }

    ACharacter* CharacterToMirror = Cast<ACharacter>(GetOwner());
    if (HasAuthority() && CharacterToMirror)
    {
        GhostTargetLocation = CharacterToMirror->GetActorLocation() + GhostOffset;
        GhostTargetRotation = CharacterToMirror->GetActorRotation();
    }

    // Smoothly interpolate toward replicated target
    float InterpSpeed = 5.f;
    SetActorLocation(FMath::VInterpTo(GetActorLocation(), GhostTargetLocation, DeltaTime, InterpSpeed));
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), GhostTargetRotation, DeltaTime, InterpSpeed));
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

    if (CharacterToMirror && CharacterToMirror->GetClass()->ImplementsInterface(UGhostMirrorSource::StaticClass()))
    {
        bIsInCameraViewLocal = IGhostMirrorSource::Execute_ShouldGhostBeVisible(CharacterToMirror);
        bShouldShow = bIsInCameraViewLocal && bPastEchoLocal;
    }
    else
    {
        bShouldShow = bPastEchoLocal;
    }

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

void AGhostCharacterActor::SetGhostRevealed_Implementation(bool bRevealed)
{
    SetIsPastEchoAbilityActive(bRevealed);
}

void AGhostCharacterActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AGhostCharacterActor, GhostTargetLocation);
    DOREPLIFETIME(AGhostCharacterActor, GhostTargetRotation);
    // bIsPastEchoAbilityActive is NOT replicated - set locally via Client RPC
}