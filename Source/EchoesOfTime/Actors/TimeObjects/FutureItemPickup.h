#pragma once

#include "CoreMinimal.h"
#include "Actors/ItemPickup.h"
#include "FutureItemPickup.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FFutureItemInvalidated, FGuid /*InstanceID*/);

UCLASS()
class ECHOESOFTIME_API AFutureItemPickup : public AItemPickup
{
    GENERATED_BODY()
public:
    AFutureItemPickup();

    virtual void BeginPlay() override;

    virtual void Interact_Implementation(AActor* Interactor) override;

    void HandleInvalidation(FGuid InvalidatedID);

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "TimeLink")
    class APastItemPickup* LinkedPastItem = nullptr;

    void OnPastItemPickedUp();

    static FFutureItemInvalidated OnFutureItemInvalidated;

    void InvalidateFromTimeline();
};