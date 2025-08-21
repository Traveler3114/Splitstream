#include "InventoryComponent.h"
#include "InventorySystem/Items/ItemBase.h" // Your item base class

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    Slots.Init(nullptr, SlotCount);
}

void UInventoryComponent::SetActiveSlot(int32 Index)
{
    if (Index >= 0 && Index < Slots.Num())
    {
        ActiveSlotIndex = Index;
        // Optionally: Broadcast equipped/changed event here
    }
}

bool UInventoryComponent::AddItem(UItemBase* Item)
{
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (Slots[i] == nullptr)
        {
            Slots[i] = Item;
            return true;
        }
    }
    return false; // Inventory full
}

void UInventoryComponent::RemoveItem(int32 Index)
{
    if (Slots.IsValidIndex(Index))
    {
        Slots[Index] = nullptr;
    }
}

UItemBase* UInventoryComponent::GetActiveItem() const
{
    return Slots.IsValidIndex(ActiveSlotIndex) ? Slots[ActiveSlotIndex] : nullptr;
}