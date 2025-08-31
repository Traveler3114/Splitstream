#include "CharacterOverlay.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "InventorySystem/ItemBase.h"
#include "InventorySystem/InventoryComponent.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Engine/Engine.h"

void UCharacterOverlay::OnInventoryChanged(const TArray<FInventorySlot>& Items)
{
    if (!InventoryBox || !LinkedInventory) return;

    InventoryBox->ClearChildren();

    for (const FInventorySlot& SlotItem : Items)
    {
        // Use the asset pointer directly now
        UItemBase* Item = SlotItem.ItemAsset;
        if (!Item || !Item->ItemIcon) continue;

        // Wrap the image in a SizeBox so we can control its size
        USizeBox* SizeBox = NewObject<USizeBox>(InventoryBox);

        UImage* ItemImage = NewObject<UImage>(SizeBox);
        ItemImage->SetBrushFromTexture(Item->ItemIcon);

        // Get texture size
        FVector2D TexSize = FVector2D(
            Item->ItemIcon->GetSizeX(),
            Item->ItemIcon->GetSizeY()
        );

        FVector2D FinalSize = TexSize * TextureScale;

        // Apply size override
        SizeBox->SetWidthOverride(FinalSize.X);
        SizeBox->SetHeightOverride(FinalSize.Y);

        // Add the image to the SizeBox
        SizeBox->AddChild(ItemImage);

        // Finally add the SizeBox to the inventory container
        UHorizontalBoxSlot* MySlot = Cast<UHorizontalBoxSlot>(InventoryBox->AddChild(SizeBox));
        if (MySlot)
        {
            // Use Auto size so it respects the box size
            MySlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
            MySlot->SetVerticalAlignment(VAlign_Center);
            MySlot->SetHorizontalAlignment(HAlign_Center);
        }
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

void UCharacterOverlay::NativeDestruct()
{
    if (LinkedInventory)
    {
        LinkedInventory->OnInventoryChanged.RemoveDynamic(this, &UCharacterOverlay::OnInventoryChanged);
    }
    Super::NativeDestruct();
}