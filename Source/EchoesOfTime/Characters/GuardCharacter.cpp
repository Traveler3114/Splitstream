#include "Actors/TimeObjects/GhostCharacterActor.h"
#include "GuardCharacter.h"
#include "Actors/NavNode.h"
#include "Engine/Engine.h"
#include "Actors/SecurityCamera.h"

AGuardCharacter::AGuardCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AGuardCharacter::BeginPlay()
{
    Super::BeginPlay();

    const bool bAuth = HasAuthority();
    if (bAuth)
    {
        FActorSpawnParameters Params;
        Params.Owner = this;
        Params.Instigator = GetInstigator();
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        SpawnedGhost = GetWorld()->SpawnActor<AGhostCharacterActor>(
            GhostClass,
            GetActorLocation(),
            GetActorRotation(),
            Params);

        if (SpawnedGhost)
        {
            SpawnedGhost->CharacterToMirror = this;
        }
    }
}

// ICameraDetectable implementation

void AGuardCharacter::OnDetectedByCamera_Implementation(ASecurityCamera* Camera)
{
    if (!bIsInCameraView)
    {
        bIsInCameraView = true;
        if (SpawnedGhost)
        {
            SpawnedGhost->UpdateGhostVisibility();
        }
    }
}

void AGuardCharacter::OnLostByCamera_Implementation(ASecurityCamera* Camera)
{
    if (bIsInCameraView)
    {
        bIsInCameraView = false;
        if (SpawnedGhost)
        {
            SpawnedGhost->UpdateGhostVisibility();
        }
    }
}

// IGhostMirrorSource implementation

bool AGuardCharacter::ShouldGhostBeVisible_Implementation() const
{
    return bIsInCameraView;
}

USkeletalMeshComponent* AGuardCharacter::GetMirrorMesh_Implementation() const
{
    return GetMesh();
}