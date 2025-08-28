#include "CharacterOverlay.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "InventorySystem/ItemBase.h"
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

void UCharacterOverlay::SetStatusText(const FString& NewStatus)
{
    if (status_txt)
    {
        status_txt->SetText(FText::FromString(NewStatus));
		status_txt->SetColorAndOpacity(NewStatus.IsEmpty() ? FLinearColor::White : FLinearColor::Red);
    }
}