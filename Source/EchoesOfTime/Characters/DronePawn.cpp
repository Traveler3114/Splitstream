#include "DronePawn.h"
#include "DefaultCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"

ADronePawn::ADronePawn()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    NetUpdateFrequency = 20.f; // Drone detection updates moderately fast

    USceneComponent* DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    DroneMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DroneMesh"));
    DroneMesh->SetupAttachment(RootComponent);

    DroneSpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("DroneSpotLight"));
    DroneSpotLight->SetupAttachment(RootComponent);
    DroneSpotLight->SetRelativeLocation(FVector(0, 0, -10));
    DroneSpotLight->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
    DroneSpotLight->SetIntensity(5000.f);
    DroneSpotLight->SetLightColor(FLinearColor::Green);
    DroneSpotLight->AttenuationRadius = DetectionDistance;
    DroneSpotLight->OuterConeAngle = ViewConeAngle * 0.5f; // Use half angle for SpotLight!
    DroneSpotLight->InnerConeAngle = ViewConeAngle * 0.5f; // Make it a hard edge to help tune detection math
    DroneSpotLight->CastShadows = false;
    DroneSpotLight->SetVisibility(true);
}

void ADronePawn::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    if (DroneSpotLight)
    {
        DroneSpotLight->OuterConeAngle = ViewConeAngle * 0.5f;
        DroneSpotLight->InnerConeAngle = ViewConeAngle * 0.5f;
        DroneSpotLight->AttenuationRadius = DetectionDistance;
    }
}

void ADronePawn::BeginPlay()
{
    Super::BeginPlay();
    GetWorldTimerManager().SetTimer(DetectionTimerHandle, this, &ADronePawn::DetectionUpdate, DetectionInterval, true);
    OnRep_DetectedActor();
}

void ADronePawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(DetectionTimerHandle);
    Super::EndPlay(EndPlayReason);
}

// Helper: Find if any bounds point is in cone
static bool IsBoundsPointInCone(
    FVector BoundsPoint, FVector ConeOrigin, FVector ConeForward,
    float ConeLength, float ConeHalfAngleDeg)
{
    FVector ToPoint = BoundsPoint - ConeOrigin;
    float Dist = ToPoint.Size();
    if (Dist > ConeLength)
        return false;
    ToPoint.Normalize();
    float Dot = FVector::DotProduct(ConeForward, ToPoint);
    float CosHalfFOV = FMath::Cos(FMath::DegreesToRadians(ConeHalfAngleDeg));
    return Dot >= CosHalfFOV;
}

void ADronePawn::DetectionUpdate()
{
    // If we've already detected someone, stay detected—no more detection checks
    if (DetectedActor)
        return;

    FVector DroneLocation = DroneSpotLight ? DroneSpotLight->GetComponentLocation() : GetActorLocation();
    FVector Forward = DroneSpotLight ? DroneSpotLight->GetForwardVector() : GetActorForwardVector();

    TArray<AActor*> OverlappedActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        DroneLocation,
        DetectionDistance,
        ObjectTypes,
        nullptr,
        TArray<AActor*>{ this },
        OverlappedActors
    );

    // Find closest DefaultCharacter with any bounds inside cone
    float ClosestDist = DetectionDistance + 1.f;
    ADefaultCharacter* NewlyDetected = nullptr;
    for (AActor* Candidate : OverlappedActors)
    {
        if (!Candidate || Candidate == this)
            continue;
        ADefaultCharacter* DefaultChar = Cast<ADefaultCharacter>(Candidate);
        if (!DefaultChar)
            continue;

        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(DefaultChar->GetRootComponent());
        FBox Bounds = PrimComp ? PrimComp->Bounds.GetBox() : FBox::BuildAABB(DefaultChar->GetActorLocation(), FVector(0,0,0));
        TArray<FVector> TestPoints;
        TestPoints.Add(Bounds.GetCenter());
        TestPoints.Add(Bounds.Min); TestPoints.Add(Bounds.Max);
        TestPoints.Add(FVector(Bounds.Min.X, Bounds.Min.Y, Bounds.Max.Z)); // 6 corners
        TestPoints.Add(FVector(Bounds.Min.X, Bounds.Max.Y, Bounds.Min.Z));
        TestPoints.Add(FVector(Bounds.Max.X, Bounds.Min.Y, Bounds.Min.Z));
        TestPoints.Add(FVector(Bounds.Min.X, Bounds.Max.Y, Bounds.Max.Z));
        TestPoints.Add(FVector(Bounds.Max.X, Bounds.Min.Y, Bounds.Max.Z));
        TestPoints.Add(FVector(Bounds.Max.X, Bounds.Max.Y, Bounds.Min.Z));
        TestPoints.Add(FVector(Bounds.Max.X, Bounds.Max.Y, Bounds.Max.Z));

        bool bDetected = false;
        for (const FVector& Point : TestPoints)
        {
            if (IsBoundsPointInCone(Point, DroneLocation, Forward, DetectionDistance, ViewConeAngle * 0.5f))
            {
                // Optional: line-of-sight check
                FHitResult Hit;
                FCollisionQueryParams Params;
                Params.AddIgnoredActor(this);
                Params.AddIgnoredActor(DefaultChar);
                bool bHit = GetWorld()->LineTraceSingleByChannel(
                    Hit, DroneLocation, Point, ECC_Visibility, Params
                );
                if (bHit && Hit.GetActor() != DefaultChar)
                    continue; // blocked

                bDetected = true;
                float Dist = (Point - DroneLocation).Size();
                if (Dist < ClosestDist)
                {
                    ClosestDist = Dist;
                    NewlyDetected = DefaultChar;
                }
                break;
            }
        }
    }

    // Only set DetectedActor ONCE!
    if (NewlyDetected)
    {
        DetectedActor = NewlyDetected;
        OnRep_DetectedActor();
    }
}

void ADronePawn::OnRep_DetectedActor()
{
    if (DroneSpotLight)
    {
        if (DetectedActor)
            DroneSpotLight->SetLightColor(FLinearColor::Red);   // Detected - red
        else
            DroneSpotLight->SetLightColor(FLinearColor::Green); // Not detected (should never run after first detection)
    }
}

void ADronePawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ADronePawn, DetectedActor);
}