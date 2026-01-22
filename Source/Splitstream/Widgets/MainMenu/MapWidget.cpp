// Fill out your copyright notice in the Description page of Project Settings.

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
    // Setup is called externally after creation
}

void UMapWidget::Setup(const FLevelData& InData)
{
    LevelData = InData;
    if (MapNameText)
        MapNameText->SetText(FText::FromString(LevelData.LevelName));
    if (MapThumbnailImage && LevelData.Thumbnail)
        MapThumbnailImage->SetBrushFromTexture(LevelData.Thumbnail);
}

void UMapWidget::OnSelect()
{
    if (OnSelectClicked.IsBound())
    {
        OnSelectClicked.Execute(LevelData.LevelName, LevelData.LevelAsset, LevelData.LobbyLevelAsset);
    }
}