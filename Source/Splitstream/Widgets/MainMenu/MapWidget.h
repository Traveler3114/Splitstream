// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MapSelectionWidget.h" // For FLevelData and Forward Decls
#include "MapWidget.generated.h"

class UVerticalBox;
class UTextBlock;
class UImage;
class UButton;

DECLARE_DELEGATE_TwoParams(FOnMapSelect, const FString&, const TSoftObjectPtr<UWorld>&);

UCLASS()
class ECHOESOFTIME_API UMapWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UPROPERTY(meta = (BindWidget))
    UVerticalBox* MapBox;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* MapNameText;

    UPROPERTY(meta = (BindWidget))
    UImage* MapThumbnailImage;

    UPROPERTY(meta = (BindWidget))
    UButton* SelectButton;

    // Store data for this widget
    FLevelData LevelData;

    // Delegate called when select is clicked
    FOnMapSelect OnSelectClicked;

    // Setup the widget given level data
    void Setup(const FLevelData& InData);

    virtual void NativeConstruct() override;

protected:
    UFUNCTION()
    void OnSelect();
};