#include "SecurityCamera.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/ArrowComponent.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"

#include "Interfaces/ICameraDetectable.h"
#include "Kismet/GameplayStatics.h"

ASecurityCamera::ASecurityCamera()
{
    PrimaryActorTick.bCanEverTick = true;
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
    // Initialize previous detected set as empty
    LastDetectedActors.Empty();
}

void ASecurityCamera::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // --- Detection logic (server only, to avoid duplicate events) ---
    if (HasAuthority())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

        TSet<AActor*> DetectedThisFrame;

        for (AActor* Actor : AllActors)
        {
            if (!Actor || !Actor->GetClass()->ImplementsInterface(UCameraDetectable::StaticClass()))
                continue;

            // Check distance
            FVector ToTarget = Actor->GetActorLocation() - GetActorLocation();
            float Distance = ToTarget.Size();
            if (Distance > DetectionDistance)
                continue;

            // Check view cone
            ToTarget.Normalize();
            FVector CameraForward = ArrowComp ? ArrowComp->GetForwardVector() : GetActorForwardVector();
            float Dot = FVector::DotProduct(CameraForward, ToTarget);
            float CosHalfFOV = FMath::Cos(FMath::DegreesToRadians(ViewConeAngle * 0.5f));
            if (Dot >= CosHalfFOV)
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
                    ICameraDetectable::Execute_OnDetectedByCamera(Actor, this);
            }
        }

        // Handle lost actors
        for (const TWeakObjectPtr<AActor>& WeakActor : LastDetectedActors)
        {
            AActor* Actor = WeakActor.Get();
            if (!Actor)
                continue;
            if (!DetectedThisFrame.Contains(Actor))
                ICameraDetectable::Execute_OnLostByCamera(Actor, this);
        }

        // Store for next frame
        LastDetectedActors.Empty();
        for (AActor* Actor : DetectedThisFrame)
            LastDetectedActors.Add(Actor);
    }

    // --- Camera panning logic (server only) ---
    if (HasAuthority() && PanSpeed > 0.0f)
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

        // Update mesh/capture rotation on the server too
        OnRep_PanOffset();
    }

    // --- Always update mesh/capture rotation on clients if PanOffset changes (handled by OnRep_PanOffset) ---

    // --- Debug drawing of camera cone and trace (all instances) ---
    if (bDrawDebug && ArrowComp && SceneCapture)
    {
        FVector Start = ArrowComp->GetComponentLocation();
        FVector Forward = ArrowComp->GetForwardVector();
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);

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
    SceneCapture->SetWorldRotation(NewRot.Add(0.0f, 90.0f, 0.0f));
}