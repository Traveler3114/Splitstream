#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/ISettingsTabInterface.h"
#include "Widgets/MainMenu/TabButton.h"
#include "DataAssets/WidgetData/SettingsTabsData.h"
#include "SettingsWidget.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
//  USettingsWidget
//
//  • Name UMG widgets "TabBar" (VerticalBox), "ContentBox" (Overlay),
//    "ApplyButton" (Button) and optionally "BackButton" (Button).
//  • Set TabButtonClass to WBP_TabButton in the Details panel.
//  • Assign a USettingsTabsData asset (DA_SettingsTabs) to TabsData — all
//    tab config lives there. No C++ or Blueprint changes needed to add tabs.
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(Abstract)
class SPLITSTREAM_API USettingsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Assign DA_SettingsTabs here. All tab entries live in that asset. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Tabs")
    TObjectPtr<USettingsTabsData> TabsData;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UVerticalBox> TabBar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UOverlay> ContentBox;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UButton> ApplyButton;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<class UButton> BackButton;

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SwitchToTab(int32 TabIndex);

    UFUNCTION(BlueprintPure, Category = "Settings")
    int32 GetActiveTabIndex() const { return ActiveTabIndex; }

protected:
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:
    void BuildTabBar();

    UFUNCTION()
    void OnTabButtonClicked(int32 TabIndex);

    UFUNCTION()
    void OnApplyClicked();

    UFUNCTION()
    void OnBackClicked();

    int32 ActiveTabIndex = -1;

    /** Kept so we can call ApplySettings on it without hardcoded casts. */
    UPROPERTY()
    TObjectPtr<UUserWidget> ActiveContentWidget;

    UPROPERTY()
    TArray<TObjectPtr<UTabButton>> TabButtons;
};
