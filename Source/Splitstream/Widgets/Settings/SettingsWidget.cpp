#include "SettingsWidget.h"

#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"

void USettingsWidget::NativePreConstruct()
{
    Super::NativePreConstruct();
    BuildTabBar();
}

void USettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BuildTabBar();

    if (ApplyButton)
    {
        ApplyButton->OnClicked.AddDynamic(this, &USettingsWidget::OnApplyClicked);
    }

    if (BackButton)
    {
        BackButton->OnClicked.AddDynamic(this, &USettingsWidget::OnBackClicked);
    }

    if (SettingsTabRegistry.Num() > 0)
    {
        SwitchToTab(0);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  BuildTabBar  —  identical pattern to UMainMenuWidget
// ─────────────────────────────────────────────────────────────────────────────
void USettingsWidget::BuildTabBar()
{
    if (!TabBar || SettingsTabRegistry.IsEmpty() || !TabButtonClass)
    {
        return;
    }

    TabBar->ClearChildren();
    TabButtons.Reset();

    for (int32 i = 0; i < SettingsTabRegistry.Num(); ++i)
    {
        const FSettingsTabEntry& Entry = SettingsTabRegistry[i];

        UTabButton* Btn = CreateWidget<UTabButton>(this, TabButtonClass);
        if (!Btn)
        {
            continue;
        }

        Btn->TabIndex = i;
        Btn->SetTabLabel(Entry.TabName);
        Btn->OnTabButtonClicked.AddDynamic(this, &USettingsWidget::OnTabButtonClicked);

        UVerticalBoxSlot* VBSlot = TabBar->AddChildToVerticalBox(Btn);
        if (VBSlot)
        {
            FSlateChildSize Fill;
            Fill.SizeRule = ESlateSizeRule::Fill;
            Fill.Value    = 1.f;
            VBSlot->SetSize(Fill);
            VBSlot->SetPadding(FMargin(4.f, 0.f));
        }

        TabButtons.Add(Btn);
    }
}

void USettingsWidget::OnTabButtonClicked(int32 TabIndex)
{
    SwitchToTab(TabIndex);
}

// ─────────────────────────────────────────────────────────────────────────────
//  SwitchToTab
// ─────────────────────────────────────────────────────────────────────────────
void USettingsWidget::SwitchToTab(int32 TabIndex)
{
    if (!SettingsTabRegistry.IsValidIndex(TabIndex) || !ContentBox)
    {
        return;
    }

    if (TabIndex == ActiveTabIndex)
    {
        return;
    }

    ContentBox->ClearChildren();
    ActiveContentWidget = nullptr;

    if (TSubclassOf<UUserWidget> WidgetClass = SettingsTabRegistry[TabIndex].TabWidgetClass)
    {
        if (APlayerController* PC = GetOwningPlayer())
        {
            UUserWidget* NewContent = CreateWidget<UUserWidget>(PC, WidgetClass);
            if (NewContent)
            {
                UOverlaySlot* OSlot = ContentBox->AddChildToOverlay(NewContent);
                if (OSlot)
                {
                    OSlot->SetHorizontalAlignment(HAlign_Fill);
                    OSlot->SetVerticalAlignment(VAlign_Fill);
                }
                ActiveContentWidget = NewContent;
            }
        }
    }

    ActiveTabIndex = TabIndex;

    for (int32 i = 0; i < TabButtons.Num(); ++i)
    {
        if (TabButtons[i])
        {
            TabButtons[i]->SetActive(i == TabIndex);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  OnApplyClicked
//  Calls ApplySettings on the active tab via the interface.
//  No hardcoded casts — works for any tab widget that implements the interface.
// ─────────────────────────────────────────────────────────────────────────────
void USettingsWidget::OnApplyClicked()
{
    if (!ActiveContentWidget)
    {
        return;
    }

    if (ActiveContentWidget->Implements<USettingsTabInterface>())
    {
        ISettingsTabInterface::Execute_ApplySettings(ActiveContentWidget);
    }
}

void USettingsWidget::OnBackClicked()
{
    SetVisibility(ESlateVisibility::Hidden);
}
