#include "CharacterOverlay.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "DataAssets/Items/ItemBase.h"
#include "ActorComponents/InventoryComponent.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Engine/Engine.h"
#include "Widgets/DetectionWidget.h"
#include "Components/Border.h"

void UCharacterOverlay::OnInventoryChanged(const TArray<FInventorySlot>& Items)
{
    if (!InventoryBox || !LinkedInventory) return;

    InventoryBox->ClearChildren();

    int32 ActiveIndex = LinkedInventory->ActiveSlotIndex;

    for (int32 i = 0; i < Items.Num(); ++i)
    {
        const FInventorySlot& SlotItem = Items[i];
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

        UBorder* Border = NewObject<UBorder>(InventoryBox);
        Border->SetBrushColor(i == ActiveIndex ? FLinearColor::Yellow : FLinearColor::White);
        Border->SetPadding(FMargin(2.0f));
        Border->SetContent(SizeBox);

        UHorizontalBoxSlot* MySlot = Cast<UHorizontalBoxSlot>(InventoryBox->AddChild(Border));
        if (MySlot)
        {
            MySlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
            MySlot->SetVerticalAlignment(VAlign_Center);
            MySlot->SetHorizontalAlignment(HAlign_Center);
        }
    }
}

void UCharacterOverlay::SetHealthText(float NewHealth)
{
    if (health_txt)
    {
        health_txt->SetText(FText::AsNumber(FMath::RoundToInt(NewHealth)));
    }
}

void UCharacterOverlay::SetPingText(float NewPing)
{
    if (ping_txt)
    {
        ping_txt->SetText(FText::FromString(FString::Printf(TEXT("%d ms"), FMath::RoundToInt(NewPing))));

        // Set color based on ping
        FSlateColor PingColor;
        if (NewPing <= 80.f)
        {
            PingColor = FSlateColor(FLinearColor::Green);
        }
        else if (NewPing <= 150.f)
        {
            PingColor = FSlateColor(FLinearColor::Yellow);
        }
        else
        {
            PingColor = FSlateColor(FLinearColor::Red);
        }
        ping_txt->SetColorAndOpacity(PingColor);
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

        if ((Progress <= 0.0f && !bIsLocked) || bIsLocked)
        {
            WidgetInstance->RemoveFromParent();
            GuardDetectionWidgets.Remove(Guard);
        }
    }
}