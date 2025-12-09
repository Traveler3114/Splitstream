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
#include "GameStates/DefaultGameState.h"

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
    if (PanSpeed <= 0.0f) return;

    const float DeltaYaw = PanSpeed * PanInterval * (bPanningRight ? 1.0f : -1.0f);
    if (PauseTimer > 0.0f)
    {
        PauseTimer -= PanInterval;
        return;
    }

    PanOffset += DeltaYaw;
    float NewYaw = CurrentYaw + PanOffset;

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
}

void ASecurityCamera::DetectionUpdate()
{
    if (!HasAuthority())
        return;

    FVector CamLoc = SceneCapture ? SceneCapture->GetComponentLocation() : GetActorLocation();

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

    for (AActor* Actor : OverlappedActors)
    {
        if (!Actor || !Actor->GetClass()->ImplementsInterface(UDetectable::StaticClass()))
            continue;

        // --- FOV/Cone Math (just like in your working version) ---
        FVector ToTarget = Actor->GetActorLocation() - CamLoc;
        float Distance = ToTarget.Size();
        if (Distance > DetectionDistance)
            continue;

        ToTarget.Normalize();
        FVector CameraForward = SceneCapture ? SceneCapture->GetForwardVector() : GetActorForwardVector();
        float Dot = FVector::DotProduct(CameraForward, ToTarget);
        float CosHalfFOV = FMath::Cos(FMath::DegreesToRadians(ViewConeAngle * 0.5f));
        if (Dot < CosHalfFOV)
            continue; // Not in cone, skip

        // --- Now do a trace for occlusion ---
        FHitResult HitResult;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        Params.AddIgnoredActor(Actor);

        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            CamLoc,
            Actor->GetActorLocation(),
            ECC_Visibility,
            Params
        );

        bool bVisible = false;
        if (bHit)
        {
            // If hit actor is the detected actor or its owner, it's visible
            if (HitResult.GetActor() == Actor || (HitResult.GetActor() && HitResult.GetActor()->GetOwner() == Actor))
                bVisible = true;
        }
        else
        {
            // No hit, nothing blocks vision (e.g. open air), so treat as visible
            bVisible = true;
        }

        if (bVisible)
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
    return LastDetectedActors.Contains(DetectingActor);
}

void ASecurityCamera::OnFullyDetected_Implementation(AActor* Detector)
{
    if (HasAuthority())
    {
        if (ADefaultGameState* GS = Cast<ADefaultGameState>(GetWorld()->GetGameState()))
        {
            GS->StartAlarm(this);
        }
    }
}