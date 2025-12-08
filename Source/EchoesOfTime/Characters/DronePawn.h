#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SpotLightComponent.h"
#include "DronePawn.generated.h"

UCLASS()
class ECHOESOFTIME_API ADronePawn : public APawn
{
    GENERATED_BODY()

public:
    ADronePawn();
    virtual void OnConstruction(const FTransform& Transform) override;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    FTimerHandle DetectionTimerHandle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionInterval = 0.2f; // seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionDistance = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float ViewConeAngle = 90.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone")
    USkeletalMeshComponent* DroneMesh;

    UPROPERTY(ReplicatedUsing=OnRep_DetectedActor, VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    AActor* DetectedActor = nullptr;

    UFUNCTION()
    void OnRep_DetectedActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone|Visual")
    USpotLightComponent* DroneSpotLight;

    // Optional debug visual
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|Debug")
    bool bDrawDebugCone = true;

    void DetectionUpdate();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
    float BaseStayChance = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
    float MinIdleTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
    float MaxIdleTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
    bool bOnlyStayOnMarkedNodes = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nodes")
    class ANavNode* CurrentNode = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nodes")
    ANavNode* PreviousNode = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nodes")
    ANavNode* NextNode = nullptr;
};