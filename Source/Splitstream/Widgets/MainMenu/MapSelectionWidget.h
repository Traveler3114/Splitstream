#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DataAssets/WidgetData/MapListData.h"
#include "MapSelectionWidget.generated.h"

class UHorizontalBox;
class UMapWidget;
class UTextBlock;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMapSelectedSignature,
    const FString&, LevelName,
    const TSoftObjectPtr<UWorld>&, LevelAsset,
    const TSoftObjectPtr<UWorld>&, LobbyLevelAsset);

UCLASS()
class SPLITSTREAM_API UMapSelectionWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // ── Config ────────────────────────────────────────────────────────────────

    /** Assign DA_Maps here. All map entries live in that asset. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maps")
    TObjectPtr<UMapListData> MapsData;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maps")
    TSubclassOf<UMapWidget> MapWidgetClass;

    // ── Tab content area ──────────────────────────────────────────────────────

    /** Horizontal list of map cards on the left */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UHorizontalBox> MapSelectionBox;

    /** Right-side detail panel — updated on hover */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UHorizontalBox> MapDetailsBox;

    // ── Detail panel widgets (inside MapDetailsBox) ───────────────────────────

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> DetailMapName;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> DetailHeistInfo;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> DetailPossibility;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> DetailThumbnail;

    // ── Events ────────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Map Selection")
    FOnMapSelectedSignature OnMapSelected;

    // ── API ───────────────────────────────────────────────────────────────────

    virtual void NativeConstruct() override;
    void PopulateLevelList();

    /** Called by UMapWidget on hover — updates the detail panel. */
    void ShowMapDetails(const FLevelData& Data);

    /** Called by UMapWidget on click — triggers session creation. */
    void MapChosen(const FString& LevelName, const TSoftObjectPtr<UWorld>& LevelAsset, const TSoftObjectPtr<UWorld>& LobbyLevelAsset);
};
