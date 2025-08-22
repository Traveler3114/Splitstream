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

    // Reference back to past item
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "TimeLink")
    class APastItemPickup* LinkedPastItem = nullptr;

    // Called by PastItem when picked up
    void OnPastItemPickedUp();
};