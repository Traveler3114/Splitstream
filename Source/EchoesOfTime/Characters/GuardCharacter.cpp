// GuardCharacter.cpp
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
#include "AbilitySystem/AttributeSets/PlayerAttributeSet.h"
#include "Controllers/DefaultPlayerController.h"
#include "GameStates/DefaultGameState.h"
#include "GameplayEffectTypes.h"
#include "TimerManager.h"

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

    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AttributeSet = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("AttributeSet"));
}

void AGuardCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
    if (GEngine)
    {
        FString DebugMsg = FString::Printf(TEXT("Health changed: %.1f"), Data.NewValue);
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, DebugMsg);
    }

    if (Data.NewValue <= 0.f)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Health is ZERO!"));
        }

        // If this guard had a pending pre-alarm, cancel it and notify the player
        if (HasAuthority())
        {
            CancelPreAlarmForDetectedPlayer();

            // If this guard already started the real alarm, cancel that as well
            if (ADefaultGameState* GS = Cast<ADefaultGameState>(GetWorld()->GetGameState()))
            {
                GS->CancelAlarm(this);
            }
        }

        if (SpawnedGhost)
        {
            SpawnedGhost->Destroy();
            SpawnedGhost = nullptr;
        }

        Destroy();
    }
}

void AGuardCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority() && AttributeInitGE)
    {
        if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
        {
            FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
            FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(AttributeInitGE, 1, EffectContext);
            if (SpecHandle.IsValid())
            {
                ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            }
        }
    }

    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
        .AddUObject(this, &AGuardCharacter::OnHealthChanged);

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

            if (UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
            {
                FGameplayTag IllegalTag = TAG_Character_Status_Illegal;

                // Handle illegal tag change
                ASC->RegisterGameplayTagEvent(IllegalTag, EGameplayTagEventType::NewOrRemoved)
                    .AddLambda([this, Player](const FGameplayTag Tag, int32 NewCount)
                        {
                            // Check if the guard is currently sensing the player
                            FActorPerceptionBlueprintInfo Info;
                            if (AIPerceptionComponent)
                                AIPerceptionComponent->GetActorsPerception(Player, Info);

                            bool bSensedNow = false;
                            for (const auto& Stimulus : Info.LastSensedStimuli)
                            {
                                if (Stimulus.WasSuccessfullySensed())
                                {
                                    bSensedNow = true;
                                    break;
                                }
                            }

                            if (bSensedNow)
                            {
                                if (NewCount > 0)
                                {
                                    DetectedActor = Player;
                                    if (GuardTimeline && TargetActor == nullptr)
                                    {
                                        GuardTimeline->Play();
                                    }
                                }
                                else
                                {
                                    if (GuardTimeline && TargetActor == nullptr)
                                    {
                                        GuardTimeline->Reverse();
                                    }
                                }
                            }
                            // If not sensed, always reverse (stop) detection
                            else
                            {
                                if (GuardTimeline && TargetActor == nullptr)
                                {
                                    GuardTimeline->Reverse();
                                }
                            }
                        });

                // Immediately check current state
                if (ASC->HasMatchingGameplayTag(IllegalTag) && bSensed)
                {
                    DetectedActor = Player;
                    if (GuardTimeline && TargetActor == nullptr)
                    {
                        GuardTimeline->Play();
                    }
                }
                // If the player has the illegal tag but is NOT sensed, reverse detection
                else if (ASC->HasMatchingGameplayTag(IllegalTag) && !bSensed)
                {
                    if (GuardTimeline && TargetActor == nullptr)
                    {
                        GuardTimeline->Reverse();
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
            if (HasAuthority())
            {
                // Start pre-alarm (server-side) and mark this guard as the instigator so we can cancel if guard is killed
                StartPreAlarmForDetectedPlayer();
            }
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
        // If a pre-alarm was pending, cancel it
        CancelPreAlarmForDetectedPlayer();

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

void AGuardCharacter::StartPreAlarmForDetectedPlayer()
{
    if (!HasAuthority() || !DetectedActor)
        return;

    // Start or restart pre-alarm server timer
    GetWorld()->GetTimerManager().ClearTimer(PreAlarmTimerHandle);
    GetWorld()->GetTimerManager().SetTimer(PreAlarmTimerHandle, this, &AGuardCharacter::OnPreAlarmTimeout, PreAlarmDuration, false);

    // Notify the detected player's controller (client) to show the pre-alarm UI
    if (ADefaultCharacter* DetectedPlayer = Cast<ADefaultCharacter>(DetectedActor))
    {
        if (ADefaultPlayerController* PC = Cast<ADefaultPlayerController>(DetectedPlayer->GetController()))
        {
            float EndTime = GetWorld()->GetTimeSeconds() + PreAlarmDuration;
            PC->ClientStartPreAlarm(EndTime);
        }
    }
}

void AGuardCharacter::CancelPreAlarmForDetectedPlayer()
{
    if (!HasAuthority())
        return;

    if (GetWorld()->GetTimerManager().IsTimerActive(PreAlarmTimerHandle))
    {
        GetWorld()->GetTimerManager().ClearTimer(PreAlarmTimerHandle);

        // Notify the detected player's controller (client) to clear the pre-alarm UI
        if (ADefaultCharacter* DetectedPlayer = Cast<ADefaultCharacter>(DetectedActor))
        {
            if (ADefaultPlayerController* PC = Cast<ADefaultPlayerController>(DetectedPlayer->GetController()))
            {
                PC->ClientCancelPreAlarm();
            }
        }
    }
}

void AGuardCharacter::OnPreAlarmTimeout()
{
    // Pre-alarm timer completed on server -> start the normal alarm
    if (HasAuthority())
    {
        if (ADefaultGameState* GS = Cast<ADefaultGameState>(GetWorld()->GetGameState()))
        {
            GS->StartAlarm(this);
        }
    }
}

void AGuardCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AGuardCharacter, bIsInCameraView);
    //DOREPLIFETIME(AGuardCharacter, TargetActor);
}