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

    if (MapThumbnailImage && LevelData.Thumbnail)
    {
        MapThumbnailImage->SetBrushFromTexture(LevelData.Thumbnail.Get());  // ← .Get()
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
