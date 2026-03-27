#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TabButton.h"
#include "MainMenuWidget.generated.h"

USTRUCT(BlueprintType)
struct FTabEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab")
    FText TabName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab")
    TSubclassOf<UUserWidget> TabWidgetClass;
};

// ─────────────────────────────────────────────────────────────────────────────
//  UMainMenuWidget
//
//  • Name UMG widgets "TabBar" (HorizontalBox) and "ContentBox" (Overlay).
//  • Set TabButtonClass to your WBP_TabButton subclass in the Details panel.
//  • Fill TabRegistry — one row per tab. No code changes needed to add tabs.
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(Abstract)
class SPLITSTREAM_API UMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Tabs")
    TArray<FTabEntry> TabRegistry;

    /**
     * Must point at a WBP subclass of UTabButton.
     * Design that widget however you like — the C++ only requires a
     * "ButtonRoot" (UButton) inside it.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Tabs")
    TSubclassOf<UTabButton> TabButtonClass;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UHorizontalBox> TabBar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UOverlay> ContentBox;

    UFUNCTION(BlueprintCallable, Category = "Main Menu")
    void SwitchToTab(int32 TabIndex);

    UFUNCTION(BlueprintPure, Category = "Main Menu")
    int32 GetActiveTabIndex() const { return ActiveTabIndex; }

protected:
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:
    void BuildTabBar();

    UFUNCTION()
    void OnTabButtonClicked(int32 TabIndex);

    int32 ActiveTabIndex = -1;

    UPROPERTY()
    TArray<TObjectPtr<UTabButton>> TabButtons;
};
