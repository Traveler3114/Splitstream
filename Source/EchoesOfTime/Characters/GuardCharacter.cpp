#include "GuardCharacter.h"
#include "Actors/TimeObjects/GhostCharacterActor.h"
#include "Actors/PointActors/NavNode.h"
#include "Engine/Engine.h"
#include "Actors/SecurityCamera.h"
#include "Characters/DefaultCharacter.h"
#include "Actors/PointActors/RefPointActor.h"
#include "Net/UnrealNetwork.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/EOTGameplayTags.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"


AGuardCharacter::AGuardCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

    SightConfig->SightRadius = 1500.0f;
    SightConfig->LoseSightRadius = 1600.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AGuardCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &AGuardCharacter::OnPerceptionUpdated);
    }
    const bool bAuth = HasAuthority();
    // GuardCharacter.cpp, in BeginPlay
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

        // Set GhostOffset immediately after spawning
        if (SpawnedGhost)
        {
            FVector Offset = ARefPointActor::GetOffsetBetweenFirstTwoRefPoints(GetWorld());
            Offset.Z -= 90.0f; // if you want manual Z adjustment
            SpawnedGhost->GhostOffset = Offset;
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

void AGuardCharacter::OnRep_IsInCameraView()
{
    if (SpawnedGhost)
    {
        SpawnedGhost->UpdateGhostVisibility();
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

void AGuardCharacter::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (ADefaultCharacter* Player = Cast<ADefaultCharacter>(Actor))
        {
            FActorPerceptionBlueprintInfo Info;
            AIPerceptionComponent->GetActorsPerception(Actor, Info);

            bool bSensed = false;
            for (const auto& Stimulus : Info.LastSensedStimuli)
            {
                if (Stimulus.WasSuccessfullySensed())
                {
                    bSensed = true;
                    break;
                }
            }

            if (bSensed)
            {
                if (UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
                {
                    FGameplayTag IllegalTag = TAG_Character_Status_Illegal;
                    ASC->RegisterGameplayTagEvent(IllegalTag, EGameplayTagEventType::NewOrRemoved)
                        .AddLambda([this, Player](const FGameplayTag Tag, int32 NewCount)
                            {
                                if (NewCount > 0)
                                {
                                    TargetActor = Player;
                                }
                                else if (TargetActor == Player)
                                {
                                    TargetActor = nullptr;
                                }
                            });

                    // Immediately check current tag state
                    if (ASC->HasMatchingGameplayTag(IllegalTag))
                    {
                        TargetActor = Player;
                    }
                }
            }
        }
    }
}

void AGuardCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AGuardCharacter, bIsInCameraView);
    //DOREPLIFETIME(AGuardCharacter, TargetActor);
}