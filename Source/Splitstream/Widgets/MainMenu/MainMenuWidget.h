#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TabButton.h"
#include "DataAssets/WidgetData/MenuTabsData.h"
#include "MainMenuWidget.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
//  UMainMenuWidget
//
//  • Name UMG widgets "TabBar" (HorizontalBox) and "ContentBox" (Overlay).
//  • Set TabButtonClass to your WBP_TabButton subclass in the Details panel.
//  • Assign a UMenuTabsData asset (DA_MainMenuTabs) to TabsData — all tab
//    config lives there. No C++ or Blueprint changes needed to add tabs.
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(Abstract)
class SPLITSTREAM_API UMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Assign DA_MainMenuTabs here. All tab entries live in that asset. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Menu|Tabs")
    TObjectPtr<UMenuTabsData> TabsData;


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
