#pragma once

#include "CoreMinimal.h"
#include "InventorySystem/Items/ItemPickup.h"
#include "PastItemPickup.generated.h"

UCLASS()
class ECHOESOFTIME_API APastItemPickup : public AItemPickup
{
    GENERATED_BODY()
public:
    APastItemPickup();

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "TimeLink")
    class AFutureItemPickup* SpawnedFutureItem = nullptr;

    virtual void BeginPlay() override;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    void SpawnLinkedFutureItem();
};