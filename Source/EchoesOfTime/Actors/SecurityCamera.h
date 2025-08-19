#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SecurityCamera.generated.h"

UCLASS()
class ECHOESOFTIME_API ASecurityCamera : public AActor
{
    GENERATED_BODY()

public:
    ASecurityCamera();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void OnConstruction(const FTransform& Transform) override;
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

    // Debug: draw vision cone and trace
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|Debug")
    bool bDrawDebug = true;
};