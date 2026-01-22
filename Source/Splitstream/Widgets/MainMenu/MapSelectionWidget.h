// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MapSelectionWidget.generated.h"

class UHorizontalBox;
class UMapWidget;

// Data struct for Levels
USTRUCT(BlueprintType)
struct FLevelData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Data")
    FString LevelName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Data")
    UTexture2D* Thumbnail;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Data")
    TSoftObjectPtr<UWorld> LevelAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Data")
    TSoftObjectPtr<UWorld> LobbyLevelAsset;
};

// Declares the delegate type for map selection events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMapSelectedSignature,
    const FString&, LevelName,
    const TSoftObjectPtr<UWorld>&, LevelAsset,
    const TSoftObjectPtr<UWorld>&, LobbyLevelAsset);

UCLASS()
class SPLITSTREAM_API UMapSelectionWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // This should be bound to a HorizontalBox in your BP
    UPROPERTY(meta = (BindWidget))
    UHorizontalBox* MapSelectionBox;

    // The Array of levels to display, settable in BP or code
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Data")
    TArray<FLevelData> LevelsData;

    // The widget class for each item (assign in BP)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Levels")
    TSubclassOf<UMapWidget> MapWidgetClass;

    // Broadcast when a map is selected
    UPROPERTY(BlueprintAssignable, Category = "Map Selection")
    FOnMapSelectedSignature OnMapSelected;

    virtual void NativeConstruct() override;
    void PopulateLevelList();
    // Called by child widgets to notify selection
    void MapChosen(const FString& LevelName, const TSoftObjectPtr<UWorld>& LevelAsset, const TSoftObjectPtr<UWorld>& LobbyLevelAsset);
};