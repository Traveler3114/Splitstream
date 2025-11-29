#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "DataAssets/ItemBase.h"
#include "ItemPickup.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPickedUp, AActor*, Interactor, UItemBase*, ItemData);

UCLASS()
class ECHOESOFTIME_API AItemPickup : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    AItemPickup();

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    UPROPERTY(BlueprintAssignable, Category = "Item")
    FOnPickedUp OnPickedUp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMeshComponent* OverrideMeshComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search", meta = (AllowPrivateAccess = true))
    class USearchComponent* SearchComp = nullptr;

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
    virtual void SetHighlighted_Implementation(bool bHighlight) override;

    UFUNCTION(BlueprintCallable, Category = "Item")
    void RefreshMeshFromItemData();

protected:
    void TryPickup(AActor* Interactor);
};