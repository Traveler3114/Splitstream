#include "CharacterOverlay.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "InventorySystem/Items/ItemBase.h"

void UCharacterOverlay::OnInventoryChanged(const TArray<class UItemBase*>& Items)
{
    if (!InventoryBox) return;

    // Debug
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("CharacterOverlay: OnInventoryChanged called, Items.Num() = %d"), Items.Num()));
    }
    UE_LOG(LogTemp, Warning, TEXT("CharacterOverlay: OnInventoryChanged called, Items.Num() = %d"), Items.Num());

    // Clear previous images
    InventoryBox->ClearChildren();

    for (UItemBase* Item : Items)
    {
        if (!Item || !Item->ItemIcon) continue;

        // Create an Image widget dynamically
        UImage* ItemImage = NewObject<UImage>(InventoryBox);
        ItemImage->SetBrushFromTexture(Item->ItemIcon);

        // Optionally set size, alignment, etc.
        // ItemImage->SetBrushSize(FVector2D(64, 64));

        InventoryBox->AddChild(ItemImage);
    }
}