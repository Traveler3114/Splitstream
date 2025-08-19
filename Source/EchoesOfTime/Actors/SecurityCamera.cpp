#include "SecurityCamera.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/ArrowComponent.h"
#include "DrawDebugHelpers.h"

ASecurityCamera::ASecurityCamera()
{
    PrimaryActorTick.bCanEverTick = true;

    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    CameraMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CameraMesh"));
    CameraMesh->SetupAttachment(DefaultSceneRoot);

    SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture2D"));
    SceneCapture->SetupAttachment(DefaultSceneRoot);

    ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
    ArrowComp->SetupAttachment(CameraMesh);
}

void ASecurityCamera::BeginPlay()
{
    Super::BeginPlay();
    CurrentYaw = 0.0f;
    bPanningRight = true;
    PauseTimer = 0.0f;
}


void ASecurityCamera::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // --- Camera panning logic with pause at ends ---
    if (PanSpeed > 0.0f)
    {
        if (PauseTimer > 0.0f)
        {
            PauseTimer -= DeltaTime;
            if (PauseTimer < 0.0f)
                PauseTimer = 0.0f;
        }
        else
        {
            float DeltaYaw = PanSpeed * DeltaTime * (bPanningRight ? 1.0f : -1.0f);
            CurrentYaw += DeltaYaw;

            if (CurrentYaw > MaxYaw)
            {
                CurrentYaw = MaxYaw;
                bPanningRight = false;
                PauseTimer = PauseAtLimit;
            }
            else if (CurrentYaw < MinYaw)
            {
                CurrentYaw = MinYaw;
                bPanningRight = true;
                PauseTimer = PauseAtLimit;
            }
        }

        // Apply rotation to CameraMesh (relative to root)
        FRotator NewRot = FRotator(0.0f, CurrentYaw, 0.0f);
        SetActorRotation(NewRot);
    }

    // --- Existing debug drawing code (unchanged) ---
    if (bDrawDebug && ArrowComp && SceneCapture)
    {
        FVector Start = ArrowComp->GetComponentLocation();
        FVector Forward = ArrowComp->GetForwardVector();
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);

        float HorizontalFOV = SceneCapture->FOVAngle;
        float AspectRatio = 1.0f;
        if (SceneCapture->TextureTarget)
        {
            AspectRatio = (float)SceneCapture->TextureTarget->SizeX / (float)SceneCapture->TextureTarget->SizeY;
        }
        float VerticalFOV = FMath::RadiansToDegrees(
            2 * FMath::Atan(FMath::Tan(FMath::DegreesToRadians(HorizontalFOV) / 2) / AspectRatio)
        );
        ViewConeAngle = HorizontalFOV;

        DrawDebugCone(
            GetWorld(),
            Start,
            Forward,
            DetectionDistance,
            FMath::DegreesToRadians(VerticalFOV * 0.5f),
            FMath::DegreesToRadians(HorizontalFOV * 0.5f),
            32,
            FColor::Green,
            false,
            0.1f,
            0,
            1.0f
        );

        FVector RayEnd = Start + Forward * DetectionDistance;
        FHitResult RayHit;
        bool bRayHit = GetWorld()->LineTraceSingleByChannel(
            RayHit, Start, RayEnd, ECC_Visibility, Params
        );
        FVector RayDrawEnd = bRayHit ? RayHit.ImpactPoint : RayEnd;
        DrawDebugLine(GetWorld(), Start, RayDrawEnd, FColor::Yellow, false, 0.1f, 0, 2.0f);
        if (bRayHit)
            DrawDebugPoint(GetWorld(), RayHit.ImpactPoint, 16.0f, FColor::Red, false, 0.1f);
    }
}

void ASecurityCamera::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    FlushPersistentDebugLines(GetWorld());

    // Reset yaw, direction, and pause for editor preview
    CurrentYaw = 0.0f;
    bPanningRight = true;
    PauseTimer = 0.0f;
    SetActorRotation(FRotator(0.0f, CurrentYaw, 0.0f));

    if (bDrawDebug && ArrowComp && SceneCapture)
    {
        FVector Start = ArrowComp->GetComponentLocation();
        FVector Forward = ArrowComp->GetForwardVector();
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);

        float HorizontalFOV = SceneCapture->FOVAngle;
        float AspectRatio = 1.0f;
        if (SceneCapture->TextureTarget)
        {
            AspectRatio = (float)SceneCapture->TextureTarget->SizeX / (float)SceneCapture->TextureTarget->SizeY;
        }
        float VerticalFOV = FMath::RadiansToDegrees(
            2 * FMath::Atan(FMath::Tan(FMath::DegreesToRadians(HorizontalFOV) / 2) / AspectRatio)
        );
        ViewConeAngle = HorizontalFOV;

        DrawDebugCone(
            GetWorld(),
            Start,
            Forward,
            DetectionDistance,
            FMath::DegreesToRadians(VerticalFOV * 0.5f),
            FMath::DegreesToRadians(HorizontalFOV * 0.5f),
            32,
            FColor::Green,
            true,
            0.0f,
            0,
            1.0f
        );

        FVector RayEnd = Start + Forward * DetectionDistance;
        FHitResult RayHit;
        bool bRayHit = GetWorld()->LineTraceSingleByChannel(
            RayHit, Start, RayEnd, ECC_Visibility, Params
        );
        FVector RayDrawEnd = bRayHit ? RayHit.ImpactPoint : RayEnd;
        DrawDebugLine(GetWorld(), Start, RayDrawEnd, FColor::Yellow, true, 0.0f, 0, 2.0f);
        if (bRayHit)
            DrawDebugPoint(GetWorld(), RayHit.ImpactPoint, 16.0f, FColor::Red, true, 0.0f);
    }
}