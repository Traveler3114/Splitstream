#pragma once

#include "CoreMinimal.h"
#include "Characters/RepairablePawn.h"
#include "AbilitySystemInterface.h"
#include "Components/SpotLightComponent.h"
#include "DronePawn.generated.h"



class UPlayerAttributeSet;

UCLASS()
class ECHOESOFTIME_API ADronePawn : public ARepairablePawn, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    ADronePawn();
    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(BlueprintReadWrite,EditAnywhere)
    bool bCanMove;
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
    virtual void OnHealthChanged(const struct FOnAttributeChangeData& Data);


    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UAbilitySystemComponent* AbilitySystemComponent;

    UPROPERTY()
    UPlayerAttributeSet* AttributeSet;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
    TSubclassOf<UGameplayEffect> AttributeInitGE;

    virtual void RequestPawnRepair(AActor* RepairInstigator) override;

    UPROPERTY(EditAnywhere,BlueprintReadWrite)
    bool bIsDead=false;

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