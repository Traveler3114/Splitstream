#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class UItemBase; // Forward declaration for your item class

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ECHOESOFTIME_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    // Configurable number of slots
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 SlotCount = 9;

    // Array of items in slots (nullptr for empty)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TArray<UItemBase*> Slots;

    // Active slot index
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 ActiveSlotIndex = 0;

    virtual void BeginPlay() override;

    // Equip/activate a slot
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SetActiveSlot(int32 Index);

    // Add item to first free slot
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(UItemBase* Item);

    // Remove item from a slot
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RemoveItem(int32 Index);

    // Get item in active slot
    UFUNCTION(BlueprintPure, Category = "Inventory")
    UItemBase* GetActiveItem() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void DropActiveItem();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TSubclassOf<UItemBase> DefaultItemClass; // Class to spawn default item
};