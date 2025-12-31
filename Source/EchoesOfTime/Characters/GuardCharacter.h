// GuardCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/IDetectable.h"
#include "Interfaces/IGhostMirrorSource.h"
#include "Interfaces/IInteractable.h"
#include "TimelineEra.h"
#include "AbilitySystemInterface.h"
#include "DataAssets/ItemBase.h"
#include "GameplayEffect.h"
#include "GuardCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGuardPickedUp, AActor*, Interactor, UItemBase*, ItemData);

class ANavNode;

UCLASS()
class ECHOESOFTIME_API AGuardCharacter : public ACharacter, public IDetectable, public IGhostMirrorSource, public IAbilitySystemInterface, public IInteractable
{
    GENERATED_BODY()

public:
    AGuardCharacter();
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void CancelInteract_Implementation(AActor* Interactor) override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;
    virtual bool IsProgressiveInteract_Implementation() override;
    UFUNCTION()
    void OnSearchComplete();
    void TryPickup(AActor* Interactor);

    UPROPERTY(BlueprintAssignable, Category = "Item")
    FOnGuardPickedUp OnGuardPickedUp;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item")
    UItemBase* ItemData;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item")
    FGuid ItemInstanceID;

    void OnHealthChanged(const struct FOnAttributeChangeData& Data);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detection")
    class UDetectionComponent* DetectionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detection")
    class USearchComponent* SearchComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard")
    bool bIsSecurityChief = false;

    UPROPERTY(ReplicatedUsing = OnRep_GuardName, EditAnywhere, BlueprintReadWrite, Category = "Guard")
    FString GuardName;

    UFUNCTION()
    void OnRep_GuardName();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard")
    UTexture2D* PortraitTexture;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guard")
    class UTextRenderComponent* NameText;

    // Assigned Locker
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard")
    class ALockerActor* AssignedLocker = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UAbilitySystemComponent* AbilitySystemComponent;

    UPROPERTY()
    class UPlayerAttributeSet* AttributeSet;

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

	bool bIsDead = false;

    UFUNCTION()
    void OnRep_IsInCameraView();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    AActor* TargetActor = nullptr;

    AActor* DetectedActor = nullptr;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ghost", meta = (AllowPrivateAccess = "true"))
    class AGhostCharacterActor* SpawnedGhost = nullptr;

    virtual void OnDetected_Implementation(AActor* Detector) override;
    virtual void OnLost_Implementation(AActor* Detector) override;
    virtual void OnForceDetectionEnd_Implementation(AActor* Detector) override;
    virtual void OnFullyDetected_Implementation(AActor* DetectingActor) override;
    virtual bool IsActorAlreadyDetected_Implementation(AActor* DetectingActor) const override;

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

    // PRE-ALARM duration for this guard
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Alarm")
    float PreAlarmDuration = 3.0f;
};