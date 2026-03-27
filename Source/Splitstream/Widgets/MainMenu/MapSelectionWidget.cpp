#include "MapSelectionWidget.h"

#include "MapWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "DefaultGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UMapSelectionWidget::NativeConstruct()
{
    Super::NativeConstruct();
    PopulateLevelList();

    if (MapDetailsBox)
    {
        MapDetailsBox->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UMapSelectionWidget::PopulateLevelList()
{
    if (!MapSelectionBox || !MapWidgetClass || !MapsData)
    {
        return;
    }

    MapSelectionBox->ClearChildren();

    for (const FLevelData& Data : MapsData->Maps)
    {
        UMapWidget* MapItem = CreateWidget<UMapWidget>(this, MapWidgetClass);
        if (!MapItem)
        {
            continue;
        }

        MapItem->Setup(Data);
        MapItem->OnMapHovered.BindUObject(this, &UMapSelectionWidget::ShowMapDetails);
        MapItem->OnSelectClicked.BindUObject(this, &UMapSelectionWidget::MapChosen);

        MapSelectionBox->AddChild(MapItem);
    }
}

void UMapSelectionWidget::ShowMapDetails(const FLevelData& Data)
{
    if (MapDetailsBox)
    {
        MapDetailsBox->SetVisibility(ESlateVisibility::Visible);
    }

    if (DetailMapName)
    {
        DetailMapName->SetText(FText::FromString(Data.LevelName));
    }

    if (DetailHeistInfo)
    {
        DetailHeistInfo->SetText(FText::FromString(Data.HeistInfo));
    }

    if (DetailPossibility)
    {
        DetailPossibility->SetText(FText::FromString(Data.Possibility));
    }

    if (DetailThumbnail && Data.Thumbnail)
    {
        DetailThumbnail->SetBrushFromTexture(Data.Thumbnail.Get());
    }
}

void UMapSelectionWidget::MapChosen(const FString& LevelName, const TSoftObjectPtr<UWorld>& LevelAsset, const TSoftObjectPtr<UWorld>& LobbyLevelAsset)
{
    OnMapSelected.Broadcast(LevelName, LevelAsset, LobbyLevelAsset);

    if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
    {
        if (UDefaultGameInstance* DGI = Cast<UDefaultGameInstance>(GI))
        {
            DGI->CreateSession(LevelName, LevelAsset, LobbyLevelAsset);
        }
    }
}
