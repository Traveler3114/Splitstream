#include "MapSelectionWidget.h"

#include "MapWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "DefaultGameInstance.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"

void UMapSelectionWidget::NativeConstruct()
{
    Super::NativeConstruct();
    PopulateLevelList();

    // Hide the detail panel until something is hovered
    if (MapDetailsBox)
    {
        MapDetailsBox->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UMapSelectionWidget::PopulateLevelList()
{
    if (!MapSelectionBox || !MapWidgetClass) return;

    MapSelectionBox->ClearChildren();

    for (const FLevelData& Data : LevelsData)
    {
        UMapWidget* MapItem = CreateWidget<UMapWidget>(this, MapWidgetClass);
        if (!MapItem) continue;

        MapItem->Setup(Data);

        // Hover → update detail panel
        MapItem->OnMapHovered.BindUObject(this, &UMapSelectionWidget::ShowMapDetails);

        // Click → travel to lobby
        MapItem->OnSelectClicked.BindUObject(this, &UMapSelectionWidget::MapChosen);

        MapSelectionBox->AddChild(MapItem);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  ShowMapDetails  —  called by the hovered map card
// ─────────────────────────────────────────────────────────────────────────────
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

// ─────────────────────────────────────────────────────────────────────────────
//  MapChosen  —  travel to the lobby level
// ─────────────────────────────────────────────────────────────────────────────
void UMapSelectionWidget::MapChosen(const FString& LevelName, const TSoftObjectPtr<UWorld>& LevelAsset, const TSoftObjectPtr<UWorld>& LobbyLevelAsset)
{
    // Broadcast so anything else listening (e.g. game mode) can react
    OnMapSelected.Broadcast(LevelName, LevelAsset, LobbyLevelAsset);
    if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
    {
        if (UDefaultGameInstance* DGI = Cast<UDefaultGameInstance>(GI))
        {
            DGI->CreateSession(LevelName, LevelAsset, LobbyLevelAsset);
        }
    }
}
