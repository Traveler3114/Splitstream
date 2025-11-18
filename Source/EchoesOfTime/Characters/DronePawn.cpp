// Fill out your copyright notice in the Description page of Project Settings.

#include "DronePawn.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/RectLightComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

ADronePawn::ADronePawn()
{
    PrimaryActorTick.bCanEverTick = false;

    USceneComponent* DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    DroneMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DroneMesh"));
    DroneMesh->SetupAttachment(RootComponent);

    // Add Rect Light (downward-facing, initially green, matches AI sight cone)
    DroneRectLight = CreateDefaultSubobject<URectLightComponent>(TEXT("DroneRectLight"));
    DroneRectLight->SetupAttachment(RootComponent);
    DroneRectLight->SetRelativeLocation(FVector(0, 0, -10));          // Slightly below the drone
    DroneRectLight->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));   // Faces straight down
    DroneRectLight->SetIntensity(5000.f);
    DroneRectLight->SetLightColor(FLinearColor::Green);
    DroneRectLight->SourceWidth = 250.f;
    DroneRectLight->SourceHeight = 250.f;
    DroneRectLight->BarnDoorAngle = 45.f;
    DroneRectLight->AttenuationRadius = 800.f;
    DroneRectLight->CastShadows = false;
    DroneRectLight->SetVisibility(true);

    // AI Perception setup
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

    SightConfig->SightRadius = 800.0f;
    SightConfig->LoseSightRadius = 900.0f;
    SightConfig->PeripheralVisionAngleDegrees = 45.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

// Sync RectLight with perception cone when constructed or edited
void ADronePawn::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (DroneRectLight && SightConfig)
    {
        // Make the light cone auto-match the perception cone
        DroneRectLight->BarnDoorAngle = SightConfig->PeripheralVisionAngleDegrees;
        DroneRectLight->AttenuationRadius = SightConfig->SightRadius;
        // Optional: You can also change rectangle size if you want wider/narrower light
        // DroneRectLight->SourceWidth = SightConfig->SightRadius * FMath::Tan(FMath::DegreesToRadians(SightConfig->PeripheralVisionAngleDegrees * 0.5f));
        // DroneRectLight->SourceHeight = DroneRectLight->SourceWidth;
    }
}

void ADronePawn::BeginPlay()
{
    Super::BeginPlay();

    // The actor rotation is handled in OnConstruction, which is called at spawn
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ADronePawn::OnPerceptionUpdated);
    }
    // Start with green light
    if (DroneRectLight)
    {
        DroneRectLight->SetLightColor(FLinearColor::Green);
    }

    // Sync RectLight with Sight on BeginPlay (covers runtime changes from BP)
    OnConstruction(GetActorTransform());
}

void ADronePawn::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{

    for (AActor* Actor : UpdatedActors)
    {
        FActorPerceptionBlueprintInfo Info;
        AIPerceptionComponent->GetActorsPerception(Actor, Info);

        for (const auto& Stimulus : Info.LastSensedStimuli)
        {
            if (Stimulus.WasSuccessfullySensed())
            {
                // Set DetectedActor and turn light red
                DetectedActor = Actor;
                if (DroneRectLight)
                    DroneRectLight->SetLightColor(FLinearColor::Red);
                break;
            }
        }
    }
}