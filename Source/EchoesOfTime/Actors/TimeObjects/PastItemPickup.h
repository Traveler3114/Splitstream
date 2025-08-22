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

    // Reference to spawned future item
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "TimeLink")
    class AFutureItemPickup* SpawnedFutureItem = nullptr;

    virtual void BeginPlay() override;
    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override; // Or override Interact, etc
	virtual void InitFromItemData(UItemBase* InItemData) override;
    // Call this to spawn the future item
    void SpawnLinkedFutureItem();
};