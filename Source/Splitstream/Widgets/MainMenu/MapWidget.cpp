#include "MapWidget.h"

#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"

void UMapWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (SelectButton)
    {
        SelectButton->OnClicked.AddDynamic(this, &UMapWidget::OnSelect);
    }
}

void UMapWidget::Setup(const FLevelData& InData)
{
    LevelData = InData;

    if (MapNameText)
    {
        MapNameText->SetText(FText::FromString(LevelData.LevelName));
    }

    if (SelectButton && LevelData.Thumbnail)
    {
        FSlateBrush Brush;
        Brush.SetResourceObject(LevelData.Thumbnail.Get());
        Brush.ImageSize = FVector2D(LevelData.Thumbnail->GetSizeX(), LevelData.Thumbnail->GetSizeY());
        Brush.DrawAs = ESlateBrushDrawType::Image;

        FSlateBrush NormalBrush = Brush;
        NormalBrush.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.5f));

        FSlateBrush HoveredBrush = Brush;
        HoveredBrush.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 1.f));

        FButtonStyle Style = SelectButton->GetStyle();
        Style.Normal = NormalBrush;
        Style.Hovered = HoveredBrush;
        SelectButton->SetStyle(Style);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  NativeOnMouseEnter  —  fires when the cursor enters this card
//  Tells the parent selection widget to update the detail panel.
// ─────────────────────────────────────────────────────────────────────────────
void UMapWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

    if (OnMapHovered.IsBound())
    {
        OnMapHovered.Execute(LevelData);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  OnSelect  —  fires when SelectButton is clicked
// ─────────────────────────────────────────────────────────────────────────────
void UMapWidget::OnSelect()
{
    if (OnSelectClicked.IsBound())
    {
        OnSelectClicked.Execute(LevelData.LevelName, LevelData.LevelAsset, LevelData.LobbyLevelAsset);
    }
}
