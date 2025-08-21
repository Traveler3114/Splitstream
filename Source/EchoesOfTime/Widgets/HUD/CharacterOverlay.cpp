#include "CharacterOverlay.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "InventorySystem/Items/ItemBase.h"
#include "InventorySystem/InventoryComponent.h"
#include "Engine/Engine.h"

void UCharacterOverlay::OnInventoryChanged(const TArray<FInventorySlot>& Items)
{
    if (!InventoryBox || !LinkedInventory) return;

    InventoryBox->ClearChildren();

    for (const FInventorySlot& SlotItem : Items)
    {
        UItemBase* Item = LinkedInventory->CreateItemInstance(SlotItem);
        if (!Item || !Item->ItemIcon) continue;

        UImage* ItemImage = NewObject<UImage>(InventoryBox);
        ItemImage->SetBrushFromTexture(Item->ItemIcon);

        InventoryBox->AddChild(ItemImage);
    }
}