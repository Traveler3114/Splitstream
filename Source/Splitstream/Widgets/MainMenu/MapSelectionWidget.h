#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Texture2D.h"          // ← ADD: needed for UTexture2D* in the struct
#include "MapSelectionWidget.generated.h"  // ← must be LAST include

class UHorizontalBox;
class UMapWidget;
class UTextBlock;
class UImage;

USTRUCT(BlueprintType)
struct FLevelData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Data")
    FString LevelName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Data")
    FString HeistInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Data")
    FString Possibility;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Data")
    TObjectPtr<UTexture2D> Thumbnail = nullptr;  // ← use TObjectPtr instead of raw pointer

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Data")
    TSoftObjectPtr<UWorld> LevelAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Data")
    TSoftObjectPtr<UWorld> LobbyLevelAsset;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMapSelectedSignature,
    const FString&, LevelName,
    const TSoftObjectPtr<UWorld>&, LevelAsset,
    const TSoftObjectPtr<UWorld>&, LobbyLevelAsset);

UCLASS()
class SPLITSTREAM_API UMapSelectionWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UHorizontalBox> MapSelectionBox;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UHorizontalBox> MapDetailsBox;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> DetailMapName;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> DetailHeistInfo;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> DetailPossibility;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> DetailThumbnail;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Data")
    TArray<FLevelData> LevelsData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
    TSubclassOf<UMapWidget> MapWidgetClass;

    UPROPERTY(BlueprintAssignable, Category = "Map Selection")
    FOnMapSelectedSignature OnMapSelected;

    virtual void NativeConstruct() override;
    void PopulateLevelList();
    void ShowMapDetails(const FLevelData& Data);
    void MapChosen(const FString& LevelName, const TSoftObjectPtr<UWorld>& LevelAsset, const TSoftObjectPtr<UWorld>& LobbyLevelAsset);
};