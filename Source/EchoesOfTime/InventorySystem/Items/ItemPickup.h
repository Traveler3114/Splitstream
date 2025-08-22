#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "ItemBase.h"
#include "ItemPickup.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPickedUp, AActor*, Interactor, UItemBase*, ItemData);

UCLASS()
class ECHOESOFTIME_API AItemPickup : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    AItemPickup();

    UPROPERTY(BlueprintAssignable, Category = "Item")
    FOnPickedUp OnPickedUp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item")
    UItemBase* ItemData;

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "Item")
    void InitFromItemData(UItemBase* InItemData);

    virtual void Interact_Implementation(AActor* Interactor) override;
};