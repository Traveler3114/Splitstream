#include "CharacterOverlay.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "InventorySystem/ItemBase.h"
#include "InventorySystem/InventoryComponent.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Engine/Engine.h"
#include "Widgets/DetectionWidget.h"

void UCharacterOverlay::OnInventoryChanged(const TArray<FInventorySlot>& Items)
{
    if (!InventoryBox || !LinkedInventory) return;

    InventoryBox->ClearChildren();

    for (const FInventorySlot& SlotItem : Items)
    {
        UItemBase* Item = SlotItem.ItemAsset;
        if (!Item || !Item->ItemIcon) continue;

        USizeBox* SizeBox = NewObject<USizeBox>(InventoryBox);

        UImage* ItemImage = NewObject<UImage>(SizeBox);
        ItemImage->SetBrushFromTexture(Item->ItemIcon);

        FVector2D TexSize = FVector2D(
            Item->ItemIcon->GetSizeX(),
            Item->ItemIcon->GetSizeY()
        );

        FVector2D FinalSize = TexSize * TextureScale;

        SizeBox->SetWidthOverride(FinalSize.X);
        SizeBox->SetHeightOverride(FinalSize.Y);

        SizeBox->AddChild(ItemImage);

        UHorizontalBoxSlot* MySlot = Cast<UHorizontalBoxSlot>(InventoryBox->AddChild(SizeBox));
        if (MySlot)
        {
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

void UCharacterOverlay::UpdateDetectionWidgetForGuard(AActor* Guard, float Progress, bool bIsLocked, float AngleDegrees)
{
    if (!Guard) return;

    UDetectionWidget*& WidgetInstance = GuardDetectionWidgets.FindOrAdd(Guard);

    if (!WidgetInstance && DetectionWidgetClass && CanvasPanel && (Progress > 0.0f || bIsLocked))
    {
        WidgetInstance = CreateWidget<UDetectionWidget>(GetWorld(), DetectionWidgetClass);
        if (WidgetInstance)
        {
            CanvasPanel->AddChild(WidgetInstance);
        }
    }

    if (WidgetInstance)
    {
        WidgetInstance->SetDetectionProgress(Progress, bIsLocked);

        UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(WidgetInstance->Slot);
        if (CanvasSlot && CanvasPanel)
        {
            FVector2D CanvasSize = CanvasPanel->GetCachedGeometry().GetLocalSize();
            FVector2D Center = CanvasSize * 0.5f;

            float MyPadding = 32.0f;
            float Radius = (FMath::Min(CanvasSize.X, CanvasSize.Y) * 0.4f) - MyPadding;

            float AngleRad = FMath::DegreesToRadians(-AngleDegrees + 90.0f);

            float WidgetX = Center.X + FMath::Cos(AngleRad) * Radius;
            float WidgetY = Center.Y - FMath::Sin(AngleRad) * Radius;

            CanvasSlot->SetPosition(FVector2D(WidgetX, WidgetY));
            CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));

            WidgetInstance->SetDetectionBarAngle(AngleDegrees);
        }

        // Remove widget if progress is zero OR chase is started (locked/full)
        if ((Progress <= 0.0f && !bIsLocked) || bIsLocked)
        {
            WidgetInstance->RemoveFromParent();
            GuardDetectionWidgets.Remove(Guard);
        }
    }
}