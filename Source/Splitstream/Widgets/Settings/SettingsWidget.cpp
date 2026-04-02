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

    if (TabsData && TabsData->Tabs.Num() > 0)
    {
        SwitchToTab(0);
    }
}

void USettingsWidget::BuildTabBar()
{
    if (!TabBar || !TabsData || TabsData->Tabs.IsEmpty())
    {
        return;
    }

    TabBar->ClearChildren();
    TabButtons.Reset();

    for (int32 i = 0; i < TabsData->Tabs.Num(); ++i)
    {
        const FSettingsTabEntry& Entry = TabsData->Tabs[i];

        UTabButton* Btn = CreateWidget<UTabButton>(this, TabsData->TabButtonClass);
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

void USettingsWidget::SwitchToTab(int32 TabIndex)
{
    if (!TabsData || !TabsData->Tabs.IsValidIndex(TabIndex) || !ContentBox)
    {
        return;
    }

    if (TabIndex == ActiveTabIndex)
    {
        return;
    }

    ContentBox->ClearChildren();
    ActiveContentWidget = nullptr;

    if (TSubclassOf<UUserWidget> WidgetClass = TabsData->Tabs[TabIndex].TabWidgetClass)
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
