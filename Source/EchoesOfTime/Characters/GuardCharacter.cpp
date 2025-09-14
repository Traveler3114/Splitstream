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
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"
#include "Controllers/DefaultPlayerController.h"

AGuardCharacter::AGuardCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

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

    GuardTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("GuardTimeline"));
}

void AGuardCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &AGuardCharacter::OnPerceptionUpdated);
    }
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
            FVector Offset = ARefPointActor::GetOffsetBetweenFirstTwoRefPoints(GetWorld());
            Offset.Z -= 90.0f;
            SpawnedGhost->GhostOffset = Offset;
        }
    }

    // Timeline setup
    if (GuardCurve)
    {
        FOnTimelineFloat ProgressFunction;
        ProgressFunction.BindUFunction(this, FName("OnTimelineFloatUpdate"));
        GuardTimeline->AddInterpFloat(GuardCurve, ProgressFunction);

        FOnTimelineEvent FinishedFunction;
        FinishedFunction.BindUFunction(this, FName("OnTimelineFinished"));
        GuardTimeline->SetTimelineFinishedFunc(FinishedFunction);

        GuardTimeline->SetLooping(false); // Do not loop, handle manually
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
                                    DetectedActor = Player;
                                    // Only show detection/progress if NOT chasing
                                    if (GuardTimeline && TargetActor == nullptr)
                                    {
                                        GuardTimeline->Play();
                                    }
                                }
                                else
                                {
                                    // Reverse detection: only reverse if timeline is not at the beginning
                                    if (GuardTimeline && TargetActor == nullptr)
                                    {
                                        GuardTimeline->Reverse();
                                    }
                                }
                            });

                    // Immediately check current tag state
                    if (ASC->HasMatchingGameplayTag(IllegalTag))
                    {
                        DetectedActor = Player;
                        if (GuardTimeline && TargetActor == nullptr)
                        {
                            GuardTimeline->Play();
                        }
                    }
                }
            }
        }
    }
}

void AGuardCharacter::OnTimelineFloatUpdate(float Value)
{
    if (DetectedActor)
    {
        ADefaultCharacter* DetectedPlayer = Cast<ADefaultCharacter>(DetectedActor);
        if (DetectedPlayer)
        {
            ADefaultPlayerController* PC = Cast<ADefaultPlayerController>(DetectedPlayer->GetController());
            if (PC)
            {
                // Calculate angle to guard
                FVector PlayerLoc = DetectedPlayer->GetActorLocation();
                FVector GuardLoc = GetActorLocation();

                // Get player camera forward and right vectors
                FRotator CameraRot = PC->PlayerCameraManager->GetCameraRotation();
                FVector CameraForward = CameraRot.Vector();
                FVector CameraRight = FRotationMatrix(CameraRot).GetUnitAxis(EAxis::Y);

                FVector ToGuard = GuardLoc - PlayerLoc;

                // Project ToGuard and CameraForward to XY plane (ignore Z)
                FVector FlatForward = CameraForward; FlatForward.Z = 0; FlatForward.Normalize();
                FVector FlatToGuard = ToGuard; FlatToGuard.Z = 0; FlatToGuard.Normalize();

                // Calculate angle in degrees between forward and direction to guard
                float AngleRad = FMath::Acos(FVector::DotProduct(FlatForward, FlatToGuard));
                float AngleDeg = FMath::RadiansToDegrees(AngleRad);

                // Determine sign: left/right
                float Sign = FVector::DotProduct(CameraRight, FlatToGuard) > 0 ? 1.0f : -1.0f;
                AngleDeg *= Sign;

                // Now pass angle to widget via controller
                PC->ClientUpdateDetectionWidgetForGuard(this, Value, false, AngleDeg);
            }
        }
    }
}

void AGuardCharacter::OnTimelineFinished()
{
    // Timeline finished = fully detected OR fully undetected (if reversed)
    if (GuardTimeline->GetPlaybackPosition() >= GuardTimeline->GetTimelineLength())
    {
        // Detection complete, start chase if not already
        if (DetectedActor && TargetActor == nullptr)
        {
            TargetActor = DetectedActor;
            // Start chase logic here!
        }

        // UI update: Progress bar full/locked
        ADefaultCharacter* DetectedPlayer = Cast<ADefaultCharacter>(DetectedActor);
        if (DetectedPlayer)
        {
            ADefaultPlayerController* PC = Cast<ADefaultPlayerController>(DetectedPlayer->GetController());
            if (PC)
            {
                // Calculate current angle
                FVector PlayerLoc = DetectedPlayer->GetActorLocation();
                FVector GuardLoc = GetActorLocation();
                FRotator CameraRot = PC->PlayerCameraManager->GetCameraRotation();
                FVector CameraForward = CameraRot.Vector();
                FVector CameraRight = FRotationMatrix(CameraRot).GetUnitAxis(EAxis::Y);
                FVector ToGuard = GuardLoc - PlayerLoc;
                FVector FlatForward = CameraForward; FlatForward.Z = 0; FlatForward.Normalize();
                FVector FlatToGuard = ToGuard; FlatToGuard.Z = 0; FlatToGuard.Normalize();
                float AngleRad = FMath::Acos(FVector::DotProduct(FlatForward, FlatToGuard));
                float AngleDeg = FMath::RadiansToDegrees(AngleRad);
                float Sign = FVector::DotProduct(CameraRight, FlatToGuard) > 0 ? 1.0f : -1.0f;
                AngleDeg *= Sign;

                PC->ClientUpdateDetectionWidgetForGuard(this, 1.0f, true, AngleDeg);
            }
        }
    }
    else if (GuardTimeline->GetPlaybackPosition() <= 0.0f)
    {
        // Timeline reversed to start, detection bar should disappear
        if (DetectedActor)
        {
            ADefaultCharacter* DetectedPlayer = Cast<ADefaultCharacter>(DetectedActor);
            if (DetectedPlayer)
            {
                ADefaultPlayerController* PC = Cast<ADefaultPlayerController>(DetectedPlayer->GetController());
                if (PC)
                {
                    // Calculate current angle
                    FVector PlayerLoc = DetectedPlayer->GetActorLocation();
                    FVector GuardLoc = GetActorLocation();
                    FRotator CameraRot = PC->PlayerCameraManager->GetCameraRotation();
                    FVector CameraForward = CameraRot.Vector();
                    FVector CameraRight = FRotationMatrix(CameraRot).GetUnitAxis(EAxis::Y);
                    FVector ToGuard = GuardLoc - PlayerLoc;
                    FVector FlatForward = CameraForward; FlatForward.Z = 0; FlatForward.Normalize();
                    FVector FlatToGuard = ToGuard; FlatToGuard.Z = 0; FlatToGuard.Normalize();
                    float AngleRad = FMath::Acos(FVector::DotProduct(FlatForward, FlatToGuard));
                    float AngleDeg = FMath::RadiansToDegrees(AngleRad);
                    float Sign = FVector::DotProduct(CameraRight, FlatToGuard) > 0 ? 1.0f : -1.0f;
                    AngleDeg *= Sign;

                    PC->ClientUpdateDetectionWidgetForGuard(this, 0.0f, false, AngleDeg);
                }
            }
        }
        // Stop the timeline completely, ready for next detection
        GuardTimeline->Stop();
    }
}

void AGuardCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AGuardCharacter, bIsInCameraView);
    //DOREPLIFETIME(AGuardCharacter, TargetActor);
}