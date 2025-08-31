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

    // REMOVED: LinkedPastItem

    void OnPastItemPickedUp();

    static FFutureItemInvalidated OnFutureItemInvalidated;

    void InvalidateFromTimeline();
};