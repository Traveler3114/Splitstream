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
}

void ASecurityCamera::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDrawDebug && ArrowComp && SceneCapture)
    {
        FVector Start = ArrowComp->GetComponentLocation();
        FVector Forward = ArrowComp->GetForwardVector();
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);

        // Get FOV from SceneCapture
        float HorizontalFOV = SceneCapture->FOVAngle;

        // Get aspect ratio from render target if available
        float AspectRatio = 1.0f;
        if (SceneCapture->TextureTarget)
        {
            AspectRatio = (float)SceneCapture->TextureTarget->SizeX / (float)SceneCapture->TextureTarget->SizeY;
        }

        // Calculate vertical FOV from horizontal FOV and aspect ratio
        float VerticalFOV = FMath::RadiansToDegrees(
            2 * FMath::Atan(FMath::Tan(FMath::DegreesToRadians(HorizontalFOV) / 2) / AspectRatio)
        );

        // Optionally, update ViewConeAngle to match SceneCapture for display
        ViewConeAngle = HorizontalFOV;

        // Draw the debug cone matching the camera's frustum
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

        // Center ray
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

    if (bDrawDebug && ArrowComp && SceneCapture)
    {
        FVector Start = ArrowComp->GetComponentLocation();
        FVector Forward = ArrowComp->GetForwardVector();
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);

        // Get FOV from SceneCapture
        float HorizontalFOV = SceneCapture->FOVAngle;

        // Get aspect ratio from render target if available
        float AspectRatio = 1.0f;
        if (SceneCapture->TextureTarget)
        {
            AspectRatio = (float)SceneCapture->TextureTarget->SizeX / (float)SceneCapture->TextureTarget->SizeY;
        }

        // Calculate vertical FOV from horizontal FOV and aspect ratio
        float VerticalFOV = FMath::RadiansToDegrees(
            2 * FMath::Atan(FMath::Tan(FMath::DegreesToRadians(HorizontalFOV) / 2) / AspectRatio)
        );

        // Optionally, update ViewConeAngle to match SceneCapture for display
        ViewConeAngle = HorizontalFOV;

        // Draw the debug cone matching the camera's frustum
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

        // Center ray
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