#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "Interfaces/IDetectable.h"
#include "Interfaces/IInteractable.h"
#include "AbilitySystemInterface.h"
#include "DataAssets/ItemBase.h"
#include "TimelineEra.h"
#include "CivilianCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCivilianPickedUp, AActor*, Interactor, UItemBase*, ItemData);

class ADeskActor;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UPlayerAttributeSet;

UCLASS()
class ECHOESOFTIME_API ACivilianCharacter : public ACharacter, public IAbilitySystemInterface, public IDetectable, public IInteractable
{
    GENERATED_BODY()

public:
    ACivilianCharacter();
    virtual void BeginPlay() override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void CancelInteract_Implementation(AActor* Interactor) override;
	virtual void SetHighlighted_Implementation(bool bHighlight) override;
    virtual bool IsProgressiveInteract_Implementation() override;
    UFUNCTION()
	void OnSearchComplete();
	void TryPickup(AActor* Interactor);

    void OnHealthChanged(const struct FOnAttributeChangeData& Data);

    UPROPERTY(BlueprintAssignable, Category = "Item")
    FOnCivilianPickedUp OnCivilianPickedUp;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item")
    UItemBase* ItemData;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item")
    FGuid ItemInstanceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian")
    FString CivilianName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian")
    UTexture2D* PortraitTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian")
    ADeskActor* AssignedDesk = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UAbilitySystemComponent* AbilitySystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detection")
    class UDetectionComponent* DetectionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detection")
    class USearchComponent* SearchComponent;

    UPROPERTY()
    UPlayerAttributeSet* AttributeSet;

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
    TSubclassOf<UGameplayEffect> AttributeInitGE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    AActor* TargetActor = nullptr;

    AActor* DetectedActor = nullptr;

	virtual void OnDetected_Implementation(AActor* Detector) override;
	virtual void OnLost_Implementation(AActor* Detector) override;

    virtual void OnFullyDetected_Implementation(AActor* DetectingActor) override;

    virtual bool IsActorAlreadyDetected_Implementation(AActor* DetectingActor) const override;

    bool bIsDead = false;

protected:
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
};