#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "Interfaces/IDetectable.h"
#include "DataAssets/ItemBase.h"
#include "ItemPickup.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemPickedUp, AActor*, Interactor, UItemBase*, ItemData);

UCLASS()
class SPLITSTREAM_API AItemPickup : public AActor, public IInteractable, public IDetectable
{
    GENERATED_BODY()

public:
    AItemPickup();

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    UPROPERTY(BlueprintAssignable, Category = "Item")
    FOnItemPickedUp OnItemPickedUp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMeshComponent* OverrideMeshComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionStimuliSourceComponent* StimuliSourceComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search", meta = (AllowPrivateAccess = true))
    class USearchComponent* SearchComp = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Detection")
    class UDetectionComponent* DetectionComponent;

    UFUNCTION()
    void OnSearchComplete();

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item")
    UItemBase* ItemData;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item")
    FGuid ItemInstanceID;

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "Item")
    void InitFromItemData(UItemBase* InItemData, FGuid InInstanceID);

    virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void CancelInteract_Implementation(AActor* Interactor) override;
    virtual bool IsProgressiveInteract_Implementation() override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;

    virtual void OnDetected_Implementation(AActor* Detector) override;
    virtual void OnLost_Implementation(AActor* Detector) override;
    virtual void OnForceDetectionEnd_Implementation(AActor* Detector) override;

    UFUNCTION(BlueprintCallable, Category = "Item")
    void RefreshMeshFromItemData();

protected:
    void TryPickup(AActor* Interactor);
};