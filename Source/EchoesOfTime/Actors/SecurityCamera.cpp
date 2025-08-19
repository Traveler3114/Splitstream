#include "SecurityCamera.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/ArrowComponent.h"
#include "DrawDebugHelpers.h"

#include "Characters/GuardCharacter.h"
#include "Actors/TimeObjects/GhostCharacterActor.h"
#include "Kismet/GameplayStatics.h"

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
    CurrentYaw = GetActorRotation().Yaw;
    PanOffset = 0.0f;
    bPanningRight = true;
    PauseTimer = 0.0f;
}

void ASecurityCamera::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // --- Guard visibility update for multi-camera support ---
    TArray<AActor*> Guards;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGuardCharacter::StaticClass(), Guards);
    for (AActor* A : Guards)
    {
        AGuardCharacter* Guard = Cast<AGuardCharacter>(A);
        if (Guard)
            Guard->bIsInCameraView = false;
    }

    for (AActor* A : Guards)
    {
        AGuardCharacter* Guard = Cast<AGuardCharacter>(A);
        if (!Guard) continue;

        FVector ToGuard = Guard->GetActorLocation() - GetActorLocation();
        float Distance = ToGuard.Size();
        if (Distance > DetectionDistance)
        {
            Guard->bIsInCameraView = false;
        }
        else
        {
            ToGuard.Normalize();
            FVector CameraForward = ArrowComp ? ArrowComp->GetForwardVector() : GetActorForwardVector();
            float Dot = FVector::DotProduct(CameraForward, ToGuard);
            float CosHalfFOV = FMath::Cos(FMath::DegreesToRadians(ViewConeAngle * 0.5f));
            Guard->bIsInCameraView = (Dot >= CosHalfFOV);
        }

        if (Guard->SpawnedGhost)
        {
            Guard->SpawnedGhost->UpdateGhostVisibility();
        }
    }

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
            PanOffset += DeltaYaw;

            if (PanOffset > MaxYaw)
            {
                PanOffset = MaxYaw;
                bPanningRight = false;
                PauseTimer = PauseAtLimit;
            }
            else if (PanOffset < MinYaw)
            {
                PanOffset = MinYaw;
                bPanningRight = true;
                PauseTimer = PauseAtLimit;
            }
        }

        // Apply rotation: initial yaw + pan offset
        FRotator NewRot = GetActorRotation();
        NewRot.Yaw = CurrentYaw + PanOffset;
        SetActorRotation(NewRot);
    }

    // --- Debug drawing of camera cone and trace ---
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

    // Reset for editor preview
    CurrentYaw = GetActorRotation().Yaw;
    PanOffset = 0.0f;
    bPanningRight = true;
    PauseTimer = 0.0f;

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