#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DataAssets/WidgetData/MapListData.h"
#include "MapWidget.generated.h"

class UTextBlock;
class UImage;
class UButton;

DECLARE_DELEGATE_ThreeParams(FOnMapSelect,
    const FString&,
    const TSoftObjectPtr<UWorld>&,
    const TSoftObjectPtr<UWorld>&);

DECLARE_DELEGATE_OneParam(FOnMapHover,
    const FLevelData&);

UCLASS()
class SPLITSTREAM_API UMapWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> MapNameText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> SelectButton;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> MapThumbnailImage;

    FLevelData LevelData;

    FOnMapSelect OnSelectClicked;
    FOnMapHover  OnMapHovered;

    void Setup(const FLevelData& InData);

    virtual void NativeConstruct() override;
    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

protected:
    UFUNCTION()
    void OnSelect();
};
