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

    // OVERRIDE!
    virtual void Interact_Implementation(AActor* Interactor) override;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "TimeLink")
    class APastItemPickup* LinkedPastItem = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost")
    FVector FutureItemOffset = FVector(0.0f, -4320.0f, 0.0f);

    void OnPastItemPickedUp();

    static FFutureItemInvalidated OnFutureItemInvalidated;

    void InvalidateFromTimeline();
};