#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Widgets/MainMenu/TabButton.h"
#include "SettingsTabsData.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
//  FSettingsTabEntry
//  One tab entry: a display label and the widget class to spawn as content.
//  The widget class should implement ISettingsTabInterface so ApplySettings
//  works without any hardcoded casts.
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FSettingsTabEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings Tab")
    FText TabName;

    /** Must implement ISettingsTabInterface to receive ApplySettings calls. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings Tab")
    TSubclassOf<UUserWidget> TabWidgetClass;
};

// ─────────────────────────────────────────────────────────────────────────────
//  USettingsTabsData
//
//  Data asset that owns the tab list for USettingsWidget.
//  Create one asset (e.g. DA_SettingsTabs) and assign it in the widget's
//  Details panel. Add / reorder / remove tabs entirely in the asset — no
//  widget Blueprint or C++ changes required.
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(BlueprintType)
class SPLITSTREAM_API USettingsTabsData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tabs")
    TSubclassOf<UTabButton> TabButtonClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tabs")
    TArray<FSettingsTabEntry> Tabs;
};
