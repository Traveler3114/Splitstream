#include "GhostCharacterActor.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Interfaces/IGhostMirrorSource.h"
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

void AGhostCharacterActor::BeginPlay()
{
    Super::BeginPlay();

    if (GhostMesh)
    {
        GhostMesh->SetVisibility(false, true);
        GhostMesh->SetHiddenInGame(false);
        GhostMesh->bOnlyOwnerSee = false;
        GhostMesh->bOwnerNoSee = false;
        UE_LOG(LogTemp, Warning, TEXT("[%s][%p] BeginPlay: Mesh visibility OFF, HiddenInGame OFF, OnlyOwnerSee=%d OwnerNoSee=%d"),
            *GetName(), this, GhostMesh->bOnlyOwnerSee, GhostMesh->bOwnerNoSee);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[%s][%p] BeginPlay: GhostMesh is nullptr!"), *GetName(), this);
    }

    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, FString::Printf(TEXT("Ghost spawned: %s [%p]"), *GetName(), this));
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
            UE_LOG(LogTemp, Warning, TEXT("[%s][%p] Tick: Mesh asset set to %s"), *GetName(), this,
                SourceMesh->GetSkeletalMeshAsset() ? *SourceMesh->GetSkeletalMeshAsset()->GetName() : TEXT("NULL"));
        }
        GhostMesh->SetLeaderPoseComponent(SourceMesh, true, true);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s][%p] Tick: SourceMesh is nullptr!"), *GetName(), this);
    }

    if (GhostMaterial && GhostMesh->GetMaterial(0) != GhostMaterial)
    {
        const int32 NumMaterials = GhostMesh->GetNumMaterials();
        for (int32 i = 0; i < NumMaterials; ++i)
        {
            GhostMesh->SetMaterial(i, GhostMaterial);
        }
        UE_LOG(LogTemp, Warning, TEXT("[%s][%p] Tick: Material set."), *GetName(), this);
    }

    if (HasAuthority())
    {
        // ... rest of your mesh/pose logic ...
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
        UE_LOG(LogTemp, Warning, TEXT("[%s][%p] UpdateGhostVisibility: SetVisibility(%d). PastEcho=%d CameraView=%d Owner=%s"),
            *GetName(), this, bShouldShow, bPastEchoLocal, bIsInCameraViewLocal, *OwnerName);
        FString MeshAssetName = GhostMesh->GetSkeletalMeshAsset() ? GhostMesh->GetSkeletalMeshAsset()->GetName() : TEXT("NULL");
        UE_LOG(LogTemp, Warning, TEXT("[%s][%p] UpdateGhostVisibility: Mesh visibility=%d, HiddenInGame=%d, MeshAsset=%s, OnlyOwnerSee=%d OwnerNoSee=%d"),
            *GetName(), this, GhostMesh->IsVisible(), GhostMesh->bHiddenInGame, *MeshAssetName, GhostMesh->bOnlyOwnerSee, GhostMesh->bOwnerNoSee);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[%s][%p] UpdateGhostVisibility: GhostMesh is nullptr!"), *GetName(), this);
    }

    FString PlayerString = FString::Printf(TEXT("Ghost: %s [%p]\nPastEcho: %s | InCameraView: %s | Show: %s\nOwner: %s"),
        *GetName(),
        this,
        bPastEchoLocal ? TEXT("TRUE") : TEXT("FALSE"),
        bIsInCameraViewLocal ? TEXT("TRUE") : TEXT("FALSE"),
        bShouldShow ? TEXT("TRUE") : TEXT("FALSE"),
        *OwnerName
    );
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            static_cast<uint64>(reinterpret_cast<uintptr_t>(this)),
            2.0f,
            bShouldShow ? FColor::Green : FColor::Red,
            PlayerString
        );
    }
}

void AGhostCharacterActor::SetIsPastEchoAbilityActive(bool bActive)
{
    UE_LOG(LogTemp, Warning, TEXT("[%s][%p] SetIsPastEchoAbilityActive(%d)"), *GetName(), this, bActive);

    bIsPastEchoAbilityActive = bActive;
    UpdateGhostVisibility();
}

// IGhostRevealable implementation
void AGhostCharacterActor::SetGhostRevealed_Implementation(bool bRevealed)
{
    UE_LOG(LogTemp, Warning, TEXT("[%s][%p] SetGhostRevealed_Implementation(%d)"), *GetName(), this, bRevealed);
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, FString::Printf(TEXT("%s [%p]: SetGhostRevealed(%s)"), *GetName(), this, bRevealed ? TEXT("TRUE") : TEXT("FALSE")));
    SetIsPastEchoAbilityActive(bRevealed);
}

void AGhostCharacterActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    // No need to replicate bIsPastEchoAbilityActive!
}