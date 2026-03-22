// AICharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/IInteractable.h"
#include "Interfaces/IDetectable.h"
#include "AbilitySystemInterface.h"
#include "DataAssets/ItemBase.h"
#include "TimelineEra.h"
#include "GameplayEffect.h"
#include "StructUtils/InstancedStruct.h"
#include "AICharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAICharacterPickedUp, AActor*, Interactor, UItemBase*, ItemData);

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UPlayerAttributeSet;
class UDetectionComponent;
class USearchComponent;

/**
 * Abstract base class for all AI-controlled characters (guards, civilians, etc.).
 *
 * Provides GAS integration (ASC + attribute set), AI perception,
 * IInteractable (search/pickup), IDetectable (detection component),
 * health attribute tracking, and item drop functionality.
 * Concrete subclasses implement specific AI behaviors via StateTree.
 */
UCLASS()
class SPLITSTREAM_API AAICharacter : public ACharacter, public IAbilitySystemInterface, public IInteractable, public IDetectable
{
    GENERATED_BODY()

public:
    AAICharacter();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // IAbilitySystemInterface
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

    // IInteractable
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void CancelInteract_Implementation(AActor* Interactor) override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;
    virtual bool IsProgressiveInteract_Implementation() override { return true; }

    // IDetectable
    virtual void OnDetected_Implementation(AActor* Detector) override;
    virtual void OnLost_Implementation(AActor* Detector) override;
    virtual void OnForceDetectionEnd_Implementation(AActor* Detector) override;
    virtual void OnFullyDetected_Implementation(AActor* ActorDetected) override;
    virtual bool IsActorAlreadyDetected_Implementation(AActor* DetectingActor) const override;

    /** Called when the character is picked up and removed */
    UPROPERTY(BlueprintAssignable, Category = "Item")
    FOnAICharacterPickedUp OnItemPickedUp;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item")
    UItemBase* ItemData;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item")
    FGuid ItemInstanceID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
    TSubclassOf<UGameplayEffect> AttributeInitGE;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UAbilitySystemComponent* AbilitySystemComponent;

    UPROPERTY()
    UPlayerAttributeSet* AttributeSet;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detection")
    UDetectionComponent* DetectionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detection")
    USearchComponent* SearchComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    AActor* TargetActor = nullptr;

    AActor* DetectedActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(BlueprintReadOnly)
    bool bIsDead = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    int32 MoneyToSubtract = -10000;

protected:
    /** Called from AttributeSet health delegate, handles death state */
    virtual void OnHealthChanged(const struct FOnAttributeChangeData& Data);

    /** Pipes WalkSpeed GAS attribute into CharacterMovement->MaxWalkSpeed */
    virtual void OnWalkSpeedChanged(const struct FOnAttributeChangeData& Data);

    UFUNCTION()
    virtual void OnSearchComplete();
    virtual void TryPickup(AActor* Interactor);

    UFUNCTION()
    virtual void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
};