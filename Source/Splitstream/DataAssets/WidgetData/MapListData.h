#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/Texture2D.h"
#include "MapListData.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
//  FLevelData
//  All data describing a single selectable map.
//  Moved here from MapSelectionWidget so it lives with the asset, not the UI.
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FLevelData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Data")
    FString LevelName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Data")
    FString HeistInfo;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Data")
    FString Possibility;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Data")
    TObjectPtr<UTexture2D> Thumbnail = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Data")
    TSoftObjectPtr<UWorld> LevelAsset;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Data")
    TSoftObjectPtr<UWorld> LobbyLevelAsset;
};

// ─────────────────────────────────────────────────────────────────────────────
//  UMapListData
//
//  Data asset that owns the list of playable maps for UMapSelectionWidget.
//  Create one asset (e.g. DA_Maps) and assign it in the widget's Details
//  panel. Add / remove / reorder maps entirely in the asset.
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(BlueprintType)
class SPLITSTREAM_API UMapListData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maps")
    TArray<FLevelData> Maps;
};
