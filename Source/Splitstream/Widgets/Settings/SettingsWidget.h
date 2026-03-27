#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/ISettingsTabInterface.h"
#include "Widgets/MainMenu/TabButton.h"
#include "SettingsWidget.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
//  FSettingsTabEntry
//  One row per tab in the Details panel — label + content widget class.
//  Add rows here to register new tabs. No C++ changes needed.
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FSettingsTabEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings Tab")
    FText TabName;

    /** Must implement ISettingsTabInterface to receive ApplySettings calls. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings Tab")
    TSubclassOf<UUserWidget> TabWidgetClass;
};

// ─────────────────────────────────────────────────────────────────────────────
//  USettingsWidget
//
//  • Name UMG widgets "TabBar" (HorizontalBox), "ContentBox" (Overlay),
//    "ApplyButton" (Button) and optionally "BackButton" (Button).
//  • Set TabButtonClass to WBP_TabButton in the Details panel.
//  • Fill SettingsTabRegistry — one row per settings tab.
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(Abstract)
class SPLITSTREAM_API USettingsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** One row per settings category (Graphics, Input, Audio...). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Tabs")
    TArray<FSettingsTabEntry> SettingsTabRegistry;

    /** Same WBP_TabButton subclass used by the main menu. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Tabs")
    TSubclassOf<UTabButton> TabButtonClass;

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

    /** Active content widget — kept so we can call ApplySettings on it. */
    UPROPERTY()
    TObjectPtr<UUserWidget> ActiveContentWidget;

    UPROPERTY()
    TArray<TObjectPtr<UTabButton>> TabButtons;
};
