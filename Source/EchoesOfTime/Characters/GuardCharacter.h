// GuardCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/IDetectable.h"
#include "Interfaces/IGhostMirrorSource.h"
#include "TimelineEra.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "GuardCharacter.generated.h"

class ANavNode;

UCLASS()
class ECHOESOFTIME_API AGuardCharacter : public ACharacter, public IDetectable, public IGhostMirrorSource, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    AGuardCharacter();

    // ============================================
    // Unreal Engine Overrides
    // ============================================
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ============================================
    // Ability System Interface
    // ============================================
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UAbilitySystemComponent* AbilitySystemComponent;

    UPROPERTY()
    class UPlayerAttributeSet* AttributeSet;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
    TSubclassOf<UGameplayEffect> AttributeInitGE;

    void OnHealthChanged(const struct FOnAttributeChangeData& Data);

    // ============================================
    // Guard Identity
    // ============================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard")
    bool bIsSecurityChief = false;

    UPROPERTY(ReplicatedUsing = OnRep_GuardName, EditAnywhere, BlueprintReadWrite, Category = "Guard")
    FString GuardName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard")
    UTexture2D* PortraitTexture;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guard")
    class UTextRenderComponent* NameText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard")
    class ALockerActor* AssignedLocker = nullptr;

    UFUNCTION()
    void OnRep_GuardName();

    // ============================================
    // Timeline & Spawn
    // ============================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    // ============================================
    // Navigation & Patrol
    // ============================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nodes")
    ANavNode* CurrentNode = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nodes")
    ANavNode* PreviousNode = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nodes")
    ANavNode* NextNode = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
    float BaseStayChance = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
    float MinIdleTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
    float MaxIdleTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
    bool bOnlyStayOnMarkedNodes = true;

    // ============================================
    // AI & Detection
    // ============================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    AActor* TargetActor = nullptr;

    AActor* DetectedActor = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Alarm")
    float PreAlarmDuration = 3.0f;

    virtual void OnDetected_Implementation(AActor* Detector) override;
    virtual void OnLost_Implementation(AActor* Detector) override;
    virtual void OnFullyDetected_Implementation(AActor* DetectingActor) override;
    virtual bool IsActorAlreadyDetected_Implementation(AActor* DetectingActor) const override;

    // ============================================
    // Ghost System
    // ============================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost")
    TSubclassOf<class AGhostCharacterActor> GhostClass;

    UPROPERTY(ReplicatedUsing = OnRep_IsInCameraView, EditAnywhere, BlueprintReadWrite, Category = "Ghost")
    bool bIsInCameraView = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ghost", meta = (AllowPrivateAccess = "true"))
    class AGhostCharacterActor* SpawnedGhost = nullptr;

    UFUNCTION()
    void OnRep_IsInCameraView();

    virtual bool ShouldGhostBeVisible_Implementation() const override;
    virtual USkeletalMeshComponent* GetMirrorMesh_Implementation() const override;

protected:
    // ============================================
    // AI Perception
    // ============================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UAISenseConfig_Sight* SightConfig;

    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
};