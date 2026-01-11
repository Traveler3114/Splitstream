// Fill out your copyright notice in the Description page of Project Settings.

#include "MapSelectionWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "MapWidget.h"

void UMapSelectionWidget::NativeConstruct()
{
    Super::NativeConstruct();
    PopulateLevelList();
}

void UMapSelectionWidget::PopulateLevelList()
{
    if (!MapSelectionBox || !MapWidgetClass) return;

    MapSelectionBox->ClearChildren();

    for (const FLevelData& LevelData : LevelsData)
    {
        UMapWidget* MapItem = CreateWidget<UMapWidget>(GetWorld(), MapWidgetClass);
        if (MapItem)
        {
            MapItem->Setup(LevelData);
            MapItem->OnSelectClicked.BindUObject(this, &UMapSelectionWidget::MapChosen);

            MapSelectionBox->AddChild(MapItem);
        }
    }
}

void UMapSelectionWidget::MapChosen(const FString& LevelName, const TSoftObjectPtr<UWorld>& LevelAsset)
{
    OnMapSelected.Broadcast(LevelName, LevelAsset);
}