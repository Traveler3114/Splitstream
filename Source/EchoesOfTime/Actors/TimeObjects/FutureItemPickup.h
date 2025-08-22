#pragma once

#include "CoreMinimal.h"
#include "InventorySystem/Items/ItemPickup.h"
#include "FutureItemPickup.generated.h"

UCLASS()
class ECHOESOFTIME_API AFutureItemPickup : public AItemPickup
{
    GENERATED_BODY()
public:
    AFutureItemPickup();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override; // <--- Add this

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "TimeLink")
    class APastItemPickup* LinkedPastItem = nullptr;

    void OnPastItemPickedUp();
};