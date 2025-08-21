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

    // --- Add this block to give a default item at game start ---
    if (GetOwner()) // Make sure we have a valid owner
    {
        // Create a new item (replace UItemBase with your item subclass if needed)
        UItemBase* NewItem = NewObject<UItemBase>(GetOwner(), DefaultItemClass);
        if (NewItem)
        {
            // Optionally set properties, e.g.:
            // NewItem->ItemName = FText::FromString("Test Item");

            AddItem(NewItem);
        }
    }
    // -----------------------------------------------------------
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
            OnInventoryChanged.Broadcast(Slots);
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
        OnInventoryChanged.Broadcast(Slots);
    }
}

UItemBase* UInventoryComponent::GetActiveItem() const
{
    return Slots.IsValidIndex(ActiveSlotIndex) ? Slots[ActiveSlotIndex] : nullptr;
}

void UInventoryComponent::DropActiveItem()
{
    UItemBase* ActiveItem = GetActiveItem();
    if (!ActiveItem) return;
    ActiveItem->OnDropped(GetOwner()); // 'GetOwner()' is usually the character
    RemoveItem(ActiveSlotIndex);
}