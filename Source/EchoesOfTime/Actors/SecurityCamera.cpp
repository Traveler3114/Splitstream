#include "SecurityCamera.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/ArrowComponent.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "Interfaces/IDetectable.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

ASecurityCamera::ASecurityCamera()
{
    PrimaryActorTick.bCanEverTick = false; // No Tick!
    bReplicates = true;

    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    CameraMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CameraMesh"));
    CameraMesh->SetupAttachment(DefaultSceneRoot);

    SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture2D"));
    SceneCapture->SetupAttachment(CameraMesh);

    ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
    ArrowComp->SetupAttachment(CameraMesh);
}

void ASecurityCamera::BeginPlay()
{
    Super::BeginPlay();
    CurrentYaw = GetActorRotation().Yaw;
    PanOffset = 0.0f;
    bPanningRight = true;
    PauseTimer = 0.0f;
    LastDetectedActors.Empty();

    // Detection timer
    GetWorldTimerManager().SetTimer(DetectionTimerHandle, this, &ASecurityCamera::DetectionUpdate, DetectionInterval, true);

    // Pan timer
    if (PanSpeed > 0.0f)
    {
        GetWorldTimerManager().SetTimer(PanTimerHandle, this, &ASecurityCamera::PanUpdate, PanInterval, true);
    }

    // Debug draw timer (optional)
    if (bDrawDebug)
    {
        GetWorldTimerManager().SetTimer(DebugDrawTimerHandle, this, &ASecurityCamera::DebugDrawUpdate, DebugDrawInterval, true);
    }
}

void ASecurityCamera::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up timers
    GetWorldTimerManager().ClearTimer(DetectionTimerHandle);
    GetWorldTimerManager().ClearTimer(PanTimerHandle);
    GetWorldTimerManager().ClearTimer(DebugDrawTimerHandle);

    Super::EndPlay(EndPlayReason);
}

void ASecurityCamera::PanUpdate()
{
    // If camera shouldn't pan, early-out
    if (PanSpeed <= 0.0f) return;

    // Use world delta seconds for smooth movement
    const float DeltaYaw = PanSpeed * PanInterval * (bPanningRight ? 1.0f : -1.0f);
    if (PauseTimer > 0.0f)
    {
        PauseTimer -= PanInterval;
        return;
    }

    PanOffset += DeltaYaw;
    float NewYaw = CurrentYaw + PanOffset;

    // Clamp between MinYaw and MaxYaw, handle direction change/pause at edges
    if (NewYaw > CurrentYaw + MaxYaw)
    {
        PanOffset = MaxYaw;
        bPanningRight = false;
        PauseTimer = PauseAtLimit;
    }
    else if (NewYaw < CurrentYaw + MinYaw)
    {
        PanOffset = MinYaw;
        bPanningRight = true;
        PauseTimer = PauseAtLimit;
    }

    OnRep_PanOffset();

    // Optionally, call MarkPackageDirty or replicate PanOffset (already handled via replication)
}

void ASecurityCamera::DetectionUpdate()
{
    if (!HasAuthority())
        return;

    FVector CamLoc = SceneCapture ? SceneCapture->GetComponentLocation() : GetActorLocation();

    // Use Sphere Overlap for broad phase
    TArray<AActor*> OverlappedActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));

    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        CamLoc,
        DetectionDistance,
        ObjectTypes,
        nullptr,
        TArray<AActor*>{ this },
        OverlappedActors
    );

    TSet<AActor*> DetectedThisFrame;
    UWorld* World = GetWorld();

    for (AActor* Actor : OverlappedActors)
    {
        if (!Actor || !Actor->GetClass()->ImplementsInterface(UDetectable::StaticClass()))
            continue;

        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
        TArray<FVector> TestPoints;

        if (PrimComp)
        {
            FBox Bounds = PrimComp->Bounds.GetBox();
            TestPoints.Add(Bounds.Min);
            TestPoints.Add(Bounds.Max);
            TestPoints.Add(FVector(Bounds.Min.X, Bounds.Min.Y, Bounds.Max.Z));
            TestPoints.Add(FVector(Bounds.Min.X, Bounds.Max.Y, Bounds.Min.Z));
            TestPoints.Add(FVector(Bounds.Max.X, Bounds.Min.Y, Bounds.Min.Z));
            TestPoints.Add(FVector(Bounds.Min.X, Bounds.Max.Y, Bounds.Max.Z));
            TestPoints.Add(FVector(Bounds.Max.X, Bounds.Min.Y, Bounds.Max.Z));
            TestPoints.Add(FVector(Bounds.Max.X, Bounds.Max.Y, Bounds.Min.Z));
        }
        else
        {
            TestPoints.Add(Actor->GetActorLocation());
        }

        bool bAnyPointDetected = false;
        for (const FVector& Point : TestPoints)
        {
            FVector ToTarget = Point - CamLoc;
            float Distance = ToTarget.Size();
            if (Distance > DetectionDistance)
                continue;

            ToTarget.Normalize();
            FVector CameraForward = SceneCapture ? SceneCapture->GetForwardVector() : GetActorForwardVector();
            float Dot = FVector::DotProduct(CameraForward, ToTarget);
            float CosHalfFOV = FMath::Cos(FMath::DegreesToRadians(ViewConeAngle * 0.5f));
            if (Dot >= CosHalfFOV)
            {
                // === LINE TRACE FOR OBSTRUCTION ===
                FHitResult HitResult;
                FCollisionQueryParams Params;
                Params.AddIgnoredActor(this);
                Params.AddIgnoredActor(Actor); // ignore self (root) for multi-part meshes

                bool bHit = World->LineTraceSingleByChannel(
                    HitResult,
                    CamLoc,
                    Point,
                    ECC_Visibility,
                    Params
                );

                // Debug draw the test line
                if (bDrawDebug)
                {
                    FColor LineColor = (bHit && HitResult.GetActor() == Actor) ? FColor::Green : FColor::Red;
                    DrawDebugLine(World, CamLoc, Point, LineColor, false, 0.2f, 0, 2.0f);
                }

                // Only detect if the trace hits the actor first (not a wall etc)
                if (bHit && HitResult.GetActor() == Actor)
                {
                    if (bDrawDebug)
                        DrawDebugPoint(World, Point, 14.f, FColor::Green, false, 0.2f);

                    bAnyPointDetected = true;

                    // Log success
                    UE_LOG(LogTemp, Verbose, TEXT("Camera [%s] CAN SEE [%s] at %s (distance %.1f)"),
                        *GetName(), *Actor->GetName(), *HitResult.Location.ToString(), Distance);

                    break;
                }
                else
                {
                    // Trace blocked, do NOT detect
                    if (bDrawDebug)
                        DrawDebugPoint(World, Point, 10.f, FColor::Red, false, 0.2f);

                    // Optional log for blocked trace
                    UE_LOG(LogTemp, VeryVerbose, TEXT("Camera [%s] LOS BLOCKED for [%s]"), *GetName(), *Actor->GetName());
                }
            }
        }

        if (bAnyPointDetected)
        {
            DetectedThisFrame.Add(Actor);
            bool bWasDetected = false;
            for (const TWeakObjectPtr<AActor>& WeakActor : LastDetectedActors)
            {
                if (WeakActor.Get() == Actor)
                {
                    bWasDetected = true;
                    break;
                }
            }
            if (!bWasDetected)
                IDetectable::Execute_OnDetected(Actor, this);
        }
    }

    // Handle lost actors
    for (const TWeakObjectPtr<AActor>& WeakActor : LastDetectedActors)
    {
        AActor* Actor = WeakActor.Get();
        if (!Actor)
            continue;
        if (!DetectedThisFrame.Contains(Actor))
            IDetectable::Execute_OnLost(Actor, this);
    }

    // Store for next frame
    LastDetectedActors.Empty();
    for (AActor* Actor : DetectedThisFrame)
        LastDetectedActors.Add(Actor);
}

void ASecurityCamera::DebugDrawUpdate()
{
    if (!(bDrawDebug && SceneCapture))
        return;

    FVector CamLoc = SceneCapture->GetComponentLocation();
    FVector CamForward = SceneCapture->GetForwardVector();
    float HorizontalFOV = SceneCapture->FOVAngle;
    float AspectRatio = 1.0f;
    if (SceneCapture->TextureTarget)
        AspectRatio = (float)SceneCapture->TextureTarget->SizeX / (float)SceneCapture->TextureTarget->SizeY;
    float VerticalFOV = FMath::RadiansToDegrees(
        2 * FMath::Atan(FMath::Tan(FMath::DegreesToRadians(HorizontalFOV) / 2) / AspectRatio)
    );
    ViewConeAngle = HorizontalFOV;

    DrawDebugCone(
        GetWorld(),
        CamLoc,
        CamForward,
        DetectionDistance,
        FMath::DegreesToRadians(VerticalFOV * 0.5f),
        FMath::DegreesToRadians(HorizontalFOV * 0.5f),
        32,
        FColor::Green,
        false,
        DebugDrawInterval,
        0,
        1.0f
    );

    FVector RayEnd = CamLoc + CamForward * DetectionDistance;
    FHitResult RayHit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bRayHit = GetWorld()->LineTraceSingleByChannel(
        RayHit, CamLoc, RayEnd, ECC_Visibility, Params
    );
    FVector RayDrawEnd = bRayHit ? RayHit.ImpactPoint : RayEnd;
    DrawDebugLine(GetWorld(), CamLoc, RayDrawEnd, FColor::Yellow, false, DebugDrawInterval, 0, 2.0f);
    if (bRayHit)
        DrawDebugPoint(GetWorld(), RayHit.ImpactPoint, 16.0f, FColor::Red, false, DebugDrawInterval);
}

void ASecurityCamera::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASecurityCamera, PanOffset);
}

void ASecurityCamera::OnRep_PanOffset()
{
    float NewYaw = CurrentYaw + PanOffset;
    FRotator NewRot = GetActorRotation();
    NewRot.Yaw = NewYaw;
    CameraMesh->SetWorldRotation(NewRot);
    if (SceneCapture)
        SceneCapture->SetWorldRotation(NewRot.Add(0.0f, 90.0f, 0.0f)); // adjust if your camera mesh needs an offset
}

bool ASecurityCamera::IsActorAlreadyDetected_Implementation(AActor* DetectingActor) const
{
    // Return true if this camera is currently detecting that actor
    return LastDetectedActors.Contains(DetectingActor);
}