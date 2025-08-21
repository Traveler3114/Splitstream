#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    Slots.SetNum(SlotCount);

    if (GetOwner()->HasAuthority() && DefaultItemClass)
    {
        AddItem(DefaultItemClass);
    }
}

void UInventoryComponent::SetActiveSlot(int32 Index)
{
    if (Index >= 0 && Index < Slots.Num())
    {
        ActiveSlotIndex = Index;
    }
}

bool UInventoryComponent::AddItem(TSubclassOf<UItemBase> ItemClass)
{
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (!Slots[i].ItemClass)
        {
            Slots[i].ItemClass = ItemClass;
            OnInventoryChanged.Broadcast(Slots);

            return true;
        }
    }
    return false;
}

void UInventoryComponent::RemoveItem(int32 Index)
{
    if (Slots.IsValidIndex(Index))
    {
        Slots[Index].ItemClass = nullptr;
        OnInventoryChanged.Broadcast(Slots);
    }
}

UItemBase* UInventoryComponent::CreateItemInstance(const FInventorySlot& Slot) const
{
    if (Slot.ItemClass)
    {
        return NewObject<UItemBase>(GetOwner(), Slot.ItemClass);
    }
    return nullptr;
}

UItemBase* UInventoryComponent::GetActiveItem() const
{
    if (Slots.IsValidIndex(ActiveSlotIndex))
    {
        return CreateItemInstance(Slots[ActiveSlotIndex]);
    }
    return nullptr;
}

void UInventoryComponent::DropActiveItem()
{
    UItemBase* ActiveItem = GetActiveItem();
    if (!ActiveItem) return;
    ActiveItem->OnDropped(GetOwner());
    RemoveItem(ActiveSlotIndex);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UInventoryComponent, Slots);
    DOREPLIFETIME(UInventoryComponent, ActiveSlotIndex);
}

void UInventoryComponent::OnRep_Slots()
{
    OnInventoryChanged.Broadcast(Slots);

}

void UInventoryComponent::OnRep_ActiveSlotIndex()
{
    // Optionally notify UI or other systems
}

void UInventoryComponent::ServerSetActiveSlot_Implementation(int32 Index)
{
    SetActiveSlot(Index);
}

void UInventoryComponent::ServerDropActiveItem_Implementation()
{
    DropActiveItem();
}

void UInventoryComponent::ServerAddItem_Implementation(TSubclassOf<UItemBase> ItemClass)
{
    AddItem(ItemClass);
}