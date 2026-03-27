#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Widgets/MainMenu/TabButton.h"
#include "MenuTabsData.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
//  FMenuTabEntry
//  One tab entry: a display label and the widget class to spawn as content.
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FMenuTabEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tab")
    FText TabName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tab")
    TSubclassOf<UUserWidget> TabWidgetClass;
};

// ─────────────────────────────────────────────────────────────────────────────
//  UMenuTabsData
//
//  Data asset that owns the tab list for UMainMenuWidget.
//  Create one asset (e.g. DA_MainMenuTabs) and assign it in the widget's
//  Details panel. Add / reorder / remove tabs entirely in the asset — no
//  widget Blueprint or C++ changes required.
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(BlueprintType)
class SPLITSTREAM_API UMenuTabsData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tabs")
    TSubclassOf<UTabButton> TabButtonClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tabs")
    TArray<FMenuTabEntry> Tabs;
};
