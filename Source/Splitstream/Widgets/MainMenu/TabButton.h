#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TabButton.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabButtonClicked, int32, TabIndex);

// ─────────────────────────────────────────────────────────────────────────────
//  UTabButton  (UUserWidget, not UButton)
//
//  Create a WBP_TabButton Blueprint subclass from this class.
//  In the Designer you can build whatever layout you want — Button, TextBlock,
//  background images, hover animations, active state visuals, etc.
//
//  Required: place a UButton named "ButtonRoot" somewhere in your widget
//  hierarchy. The C++ binds OnClicked on it automatically.
//
//  The menu calls SetTabLabel() and SetActive() — wire those up in Blueprint
//  via the NativeOnSetLabel / NativeOnSetActive events if you need custom
//  visual responses beyond the defaults.
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(Abstract)
class SPLITSTREAM_API UTabButton : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Assigned by the menu. Read-only — do not set manually. */
    UPROPERTY(BlueprintReadOnly, Category = "Tab Button")
    int32 TabIndex = -1;

    /** Fired when the button is clicked — carries the tab index. */
    UPROPERTY(BlueprintAssignable, Category = "Tab Button")
    FOnTabButtonClicked OnTabButtonClicked;

    /** Called by the menu to set the label text. */
    UFUNCTION(BlueprintCallable, Category = "Tab Button")
    void SetTabLabel(const FText& InLabel);

    /** Called by the menu whenever the active tab changes. */
    UFUNCTION(BlueprintCallable, Category = "Tab Button")
    void SetActive(bool bActive);

    UFUNCTION(BlueprintPure, Category = "Tab Button")
    bool IsActiveTab() const { return bIsActive; }

protected:
    /** The clickable button inside your WBP layout. Must be named "ButtonRoot". */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UButton> ButtonRoot;

    /** Optional: TextBlock to auto-populate with the tab name. Name it "LabelText". */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<class UTextBlock> LabelText;

    virtual void NativeConstruct() override;

    /**
     * Override in Blueprint to react to label changes —
     * e.g. update a custom RichTextBlock or play an animation.
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Tab Button")
    void OnLabelSet(const FText& InLabel);

    /**
     * Override in Blueprint to react to active state changes —
     * e.g. swap a style, play an animation, change colors.
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Tab Button")
    void OnActiveStateChanged(bool bNowActive);

private:
    bool bIsActive = false;

    UFUNCTION()
    void HandleClicked();
};
