#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimelineEra.h"
#include "Interfaces/IDetectable.h"
#include "SecurityCamera.generated.h"

UCLASS()
class SPLITSTREAM_API ASecurityCamera : public AActor, public IDetectable
{
    GENERATED_BODY()

public:
    ASecurityCamera();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void OnFullyDetected_Implementation(AActor* DetectingActor) override;
    virtual bool IsActorAlreadyDetected_Implementation(AActor* DetectingActor) const override;

    // Timer handles
    FTimerHandle DetectionTimerHandle;
    FTimerHandle PanTimerHandle;

    // Timer intervals (can be tweaked)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float DetectionInterval = 0.2f; // seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float PanInterval = 0.02f; // seconds

    // Timer callbacks
    void DetectionUpdate();
    void PanUpdate();

public:
    // Replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
    USceneComponent* DefaultSceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
    UStaticMeshComponent* CameraMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
    class USceneCaptureComponent2D* SceneCapture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    class UArrowComponent* ArrowComp;

    // Detection parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float ViewConeAngle = 90.0f; // degrees (full angle)

    // Camera rotation (pan) settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Pan")
    float MinYaw = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Pan")
    float MaxYaw = 45.0f;

    // If PanSpeed = 0, camera will not pan
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Pan")
    float PanSpeed = 30.0f; // degrees per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Pan")
    float PauseAtLimit = 2.0f; // seconds to pause at each end

    // Debug option: Show detection cone in editor/runtime
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawDebugDetectionCone = false;

private:
    float CurrentYaw = 0.0f;
    bool bPanningRight = true;
    float PauseTimer = 0.0f;
    UPROPERTY(ReplicatedUsing = OnRep_PanOffset)
    float PanOffset = 0.0f;

    UFUNCTION()
    void OnRep_PanOffset();

    // Keep track of actors detected last frame for state transitions
    TSet<TWeakObjectPtr<AActor>> LastDetectedActors;
};