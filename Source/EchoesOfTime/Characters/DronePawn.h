#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemInterface.h"
#include "Components/SpotLightComponent.h"
#include "TimelineEra.h"
#include "GameplayEffect.h"
#include "Interfaces/IRepairable.h"
#include "DronePawn.generated.h"

class UPlayerAttributeSet;

UCLASS()
class ECHOESOFTIME_API ADronePawn : public APawn, public IAbilitySystemInterface, public IRepairable
{
    GENERATED_BODY()
public:
    ADronePawn();
    virtual void OnConstruction(const FTransform& Transform) override;

    // IAbilitySystemInterface
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

    // IRepairable interface (all required implementations)
    virtual void RequestRepair_Implementation(AActor* RepairInstigator) override;
    virtual float GetRepairTime_Implementation() const override { return RepairTime; }
    virtual ETimelineEra GetTimelineEra_Implementation() const override { return TimelineEra; }
    virtual AActor* GetCompletionTarget_Implementation() const override { return const_cast<ADronePawn*>(this); }
    virtual FOnRepairRequested& GetOnRepairRequested() override { return OnRepairRequested; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair")
    float RepairTime = 5.0f;

    UPROPERTY(BlueprintAssignable, Category = "Repair")
    FOnRepairRequested OnRepairRequested;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void OnHealthChanged(const struct FOnAttributeChangeData& Data);

    void DetectionUpdate();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UAbilitySystemComponent* AbilitySystemComponent;

    UPROPERTY()
    UPlayerAttributeSet* AttributeSet;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
    TSubclassOf<UGameplayEffect> AttributeInitGE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsDead = false;

    FTimerHandle DetectionTimerHandle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionInterval = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionDistance = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float ViewConeAngle = 90.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone")
    USkeletalMeshComponent* DroneMesh;

    UPROPERTY(ReplicatedUsing = OnRep_DetectedActor, VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    AActor* DetectedActor = nullptr;

    UFUNCTION()
    void OnRep_DetectedActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone|Visual")
    USpotLightComponent* DroneSpotLight;

    // --- Smooth mesh alignment ---
    FTimerHandle MeshAlignTimerHandle;
    FVector MeshAlignStartLocation;
    FRotator MeshAlignStartRotation;
    FVector MeshAlignTargetLocation;
    FRotator MeshAlignTargetRotation;
    float MeshAlignElapsed = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone|Visual")
    float MeshAlignDuration = 0.5f; // Blend duration in seconds (tweak as desired)
    UFUNCTION()
    void UpdateMeshAlignInterp();

    // Idle options and navigation nodes
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