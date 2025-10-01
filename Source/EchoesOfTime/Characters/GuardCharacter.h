#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/ICameraDetectable.h"
#include "Interfaces/IGhostMirrorSource.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"
#include "AbilitySystemInterface.h"
#include "GuardCharacter.generated.h"

class ANavNode;

UCLASS()
class ECHOESOFTIME_API AGuardCharacter : public ACharacter, public ICameraDetectable, public IGhostMirrorSource, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    AGuardCharacter();
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    //UFUNCTION()
    void OnHealthChanged(const struct FOnAttributeChangeData& Data);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UAbilitySystemComponent* AbilitySystemComponent;

    // Attribute Set
    UPROPERTY()
    class UPlayerAttributeSet* AttributeSet;

    // IAbilitySystemInterface
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }


    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
    TSubclassOf<UGameplayEffect> AttributeInitGE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
    float BaseStayChance = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
    float MinIdleTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
    float MaxIdleTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
    bool bOnlyStayOnMarkedNodes = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nodes")
    ANavNode* CurrentNode = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nodes")
    ANavNode* PreviousNode = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nodes")
    ANavNode* NextNode = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost")
    TSubclassOf<class AGhostCharacterActor> GhostClass;

    UPROPERTY(ReplicatedUsing = OnRep_IsInCameraView, EditAnywhere, BlueprintReadWrite, Category = "Ghost")
    bool bIsInCameraView = false;

    UFUNCTION()
    void OnRep_IsInCameraView();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    AActor* TargetActor = nullptr;

	AActor* DetectedActor = nullptr;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ghost", meta = (AllowPrivateAccess = "true"))
    class AGhostCharacterActor* SpawnedGhost = nullptr;

    // Timeline
    UPROPERTY()
    UTimelineComponent* GuardTimeline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
    UCurveFloat* GuardCurve;

    UFUNCTION()
    void OnTimelineFloatUpdate(float Value);

    UFUNCTION()
    void OnTimelineFinished();

    // ICameraDetectable implementation
    virtual void OnDetectedByCamera_Implementation(class ASecurityCamera* Camera) override;
    virtual void OnLostByCamera_Implementation(class ASecurityCamera* Camera) override;

    // IGhostMirrorSource implementation
    virtual bool ShouldGhostBeVisible_Implementation() const override;
    virtual USkeletalMeshComponent* GetMirrorMesh_Implementation() const override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UAISenseConfig_Sight* SightConfig;

    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
};