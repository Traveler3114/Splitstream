#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsWidget.generated.h"

class UButton;
class UTextBlock;
class UWidgetSwitcher;
class UGraphicsWidget;
class UInputWidget;

UCLASS()
class SPLITSTREAM_API USettingsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

protected:
    // --- Tab Buttons ---
    UPROPERTY(meta = (BindWidget))
    UButton* GraphicsTabButton;

    UPROPERTY(meta = (BindWidget))
    UButton* InputTabButton;

    // --- Tab Labels (optional) ---
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* GraphicsTabLabel;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* InputTabLabel;

    // --- Widget Switcher Container for Tabs ---
    UPROPERTY(meta = (BindWidget))
    UWidgetSwitcher* SettingsSwitcher;

    // --- Subwidgets ---
    UPROPERTY(meta = (BindWidget))
    UGraphicsWidget* GraphicsWidget;

    UPROPERTY(meta = (BindWidget))
    UInputWidget* InputWidget;

    // --- Apply/Back (if you want main apply/cancel at top level) ---
    UPROPERTY(meta = (BindWidget))
    UButton* ApplyButton;

    // --- Tab Switch Handlers ---
    UFUNCTION()
    void OnGraphicsTabClicked();
    UFUNCTION()
    void OnInputTabClicked();

    // --- (Optional) Apply/Back Handlers ---
    UFUNCTION()
    void OnApplyClicked();
    UFUNCTION()
    void OnBackClicked();

    // -- Utility --
    void UpdateTabHighlight();
};