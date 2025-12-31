// CharacterOverlay.cpp
#include "CharacterOverlay.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "DataAssets/ItemBase.h"
#include "ActorComponents/InventoryComponent.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/VerticalBox.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Engine/Engine.h"
#include "Widgets/DetectionWidget.h"
#include "Components/Border.h"
#include "Blueprint/WidgetLayoutLibrary.h" 

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

void UCharacterOverlay::UpdateRepairCountdowns(const TMap<AActor*, float>& NewEndTimes)
{
    CompletionTargetRepairEndTimes = NewEndTimes;

    if (!RepairCountdownsBox) return;

    RepairCountdownsBox->ClearChildren();

    float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

    for (const auto& Elem : CompletionTargetRepairEndTimes)
    {
        AActor* Target = Elem.Key;
        float EndTime = Elem.Value;
        float Remaining = FMath::Max(0.f, EndTime - Now);

        FString TargetName = Target ? Target->GetName() : FString("Unknown");
        FString CountdownText = FString::Printf(TEXT("%.0f seconds until %s is fixed"), Remaining, *TargetName);

        UTextBlock* RepairText = NewObject<UTextBlock>(RepairCountdownsBox);
        RepairText->SetText(FText::FromString(CountdownText));

        RepairCountdownsBox->AddChildToVerticalBox(RepairText);
    }
}

void UCharacterOverlay::SetObjectiveText(int32 Current, int32 Target)
{
    if (objective_txt)
    {
        objective_txt->SetText(FText::FromString(
            FString::Printf(TEXT("Collected money: %d/%d"), Current, Target)
        ));
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
    // Keep old behavior: white when empty, red when non-empty
    if (status_txt)
    {
        status_txt->SetText(FText::FromString(NewStatus));
        status_txt->SetColorAndOpacity(NewStatus.IsEmpty() ? FLinearColor::White : FLinearColor::Red);
    }
}

void UCharacterOverlay::SetStatusTextWithColor(const FString& NewStatus, const FLinearColor& Color)
{
    if (status_txt)
    {
        status_txt->SetText(FText::FromString(NewStatus));
        status_txt->SetColorAndOpacity(Color);
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


void UCharacterOverlay::UpdateDetectionWidget(AActor* Detector, float Progress, bool bIsLocked, FVector2D ScreenPosition)
{
    if (!CanvasPanel || !Detector) return;

    UDetectionWidget*& Widget = DetectionWidgets.FindOrAdd(Detector);
    if (!Widget)
    {
        if (!DetectionWidgetClass) return;
        Widget = CreateWidget<UDetectionWidget>(GetWorld(), DetectionWidgetClass);
        if (Widget)
        {
            CanvasPanel->AddChild(Widget);
        }
    }

    if (Widget)
    {
        Widget->SetDetectionProgress(Progress, bIsLocked);

        FVector2D CanvasSize = CanvasPanel->GetCachedGeometry().GetLocalSize();
        FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this);

        FVector2D LocalPosition = ScreenPosition;
        // Only scale if sizes are not (nearly) equal
        if (!ViewportSize.IsNearlyZero() && !CanvasSize.IsNearlyZero()
            && !CanvasSize.Equals(ViewportSize, 1.0f)) // 1 pixel tolerance
        {
            LocalPosition.X = (ScreenPosition.X / ViewportSize.X) * CanvasSize.X;
            LocalPosition.Y = (ScreenPosition.Y / ViewportSize.Y) * CanvasSize.Y;
        }

        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot))
        {
            CanvasSlot->SetPosition(LocalPosition);
            CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
        }

        if (Progress <= 0.001f && !bIsLocked)
        {
            Widget->RemoveFromParent();
            DetectionWidgets.Remove(Detector);
        }
    }
}