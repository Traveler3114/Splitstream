#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.h"
#include "ItemPickup.generated.h"

/**
 * AItemPickup
 *
 * Actor that represents a physical item in the world for the player to pick up.
 * Holds a reference to the inventory item data (UItemBase).
 */
UCLASS()
class ECHOESOFTIME_API AItemPickup : public AActor
{
    GENERATED_BODY()

public:
    AItemPickup();

    // Mesh for visual representation in the world
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* MeshComponent;

    // Reference to the inventory data object
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item")
    UItemBase* ItemData;

    virtual void BeginPlay() override;

    // Call this after spawning to initialize visuals from ItemData
    UFUNCTION(BlueprintCallable, Category = "Item")
    void InitFromItemData(UItemBase* InItemData);
};