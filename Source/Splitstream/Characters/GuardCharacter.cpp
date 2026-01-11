#include "GuardCharacter.h"
#include "Actors/TimeObjects/GhostCharacterActor.h"
#include "Engine/Engine.h"
#include "Components/TextRenderComponent.h"
#include "Actors/PointActors/RefPointActor.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "GameStates/DefaultGameState.h"
#include "Components/StateTreeComponent.h"

AGuardCharacter::AGuardCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    NameText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NameText"));
    NameText->SetupAttachment(GetMesh());
}

void AGuardCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (NameText)
    {
        NameText->SetText(FText::FromString(GuardName));
    }
    if (HasAuthority())
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
            FVector Offset = ARefPointActor::GetOffsetBetweenFirstTwoRefPoints(GetWorld());
            Offset.Z -= 90.0f;
            SpawnedGhost->GhostOffset = Offset;
        }
    }
}

void AGuardCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
    Super::OnHealthChanged(Data);
    if (Data.NewValue <= 0.f)
    {
        if (HasAuthority())
        {
            if (ADefaultGameState* GS = Cast<ADefaultGameState>(GetWorld()->GetGameState()))
            {
                GS->RemovePreAlarmInstigator(this);
            }
        }
        if (NameText)
        {
            NameText->SetVisibility(false);
        }
        if (SpawnedGhost)
        {
            SpawnedGhost->Destroy();
            SpawnedGhost = nullptr;
        }
    }
}

void AGuardCharacter::OnRep_GuardName()
{
    if (NameText)
        NameText->SetText(FText::FromString(GuardName));
}

void AGuardCharacter::OnDetected_Implementation(AActor* Detector)
{
    Super::OnDetected_Implementation(Detector);
    bIsInCameraView = true;
    if (SpawnedGhost)
    {
        SpawnedGhost->UpdateGhostVisibility();
    }
}

void AGuardCharacter::OnLost_Implementation(AActor* Detector)
{
    Super::OnLost_Implementation(Detector);
    bIsInCameraView = false;
    if (SpawnedGhost)
    {
        SpawnedGhost->UpdateGhostVisibility();
    }
}

void AGuardCharacter::OnFullyDetected_Implementation(AActor* ActorDetected)
{
    Super::OnFullyDetected_Implementation(ActorDetected);
    if (HasAuthority())
    {
        if (ADefaultGameState* GS = Cast<ADefaultGameState>(GetWorld()->GetGameState()))
        {
            GS->StartPreAlarm(this, PreAlarmDuration);
        }
    }
}

void AGuardCharacter::OnRep_IsInCameraView()
{
    if (SpawnedGhost)
    {
        SpawnedGhost->UpdateGhostVisibility();
    }
}

bool AGuardCharacter::ShouldGhostBeVisible_Implementation() const
{
    return bIsInCameraView;
}

USkeletalMeshComponent* AGuardCharacter::GetMirrorMesh_Implementation() const
{
    return GetMesh();
}

void AGuardCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AGuardCharacter, GuardName);
    DOREPLIFETIME(AGuardCharacter, bIsInCameraView);
}