#include "MainMenuWidget.h"

#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"

void UMainMenuWidget::NativePreConstruct()
{
    Super::NativePreConstruct();
    BuildTabBar();
}

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BuildTabBar();

    if (TabsData && TabsData->Tabs.Num() > 0)
    {
        SwitchToTab(0);
    }
}

void UMainMenuWidget::BuildTabBar()
{
    if (!TabBar || !TabsData || TabsData->Tabs.IsEmpty())
    {
        return;
    }

    TabBar->ClearChildren();
    TabButtons.Reset();

    for (int32 i = 0; i < TabsData->Tabs.Num(); ++i)
    {
        const FMenuTabEntry& Entry = TabsData->Tabs[i];

        UTabButton* Btn = CreateWidget<UTabButton>(this, TabsData->TabButtonClass);
        if (!Btn)
        {
            continue;
        }

        Btn->TabIndex = i;
        Btn->SetTabLabel(Entry.TabName);
        Btn->OnTabButtonClicked.AddDynamic(this, &UMainMenuWidget::OnTabButtonClicked);

        UHorizontalBoxSlot* HBSlot = TabBar->AddChildToHorizontalBox(Btn);
        if (HBSlot)
        {
            FSlateChildSize Fill;
            Fill.SizeRule = ESlateSizeRule::Fill;
            Fill.Value    = 1.f;
            HBSlot->SetSize(Fill);
            HBSlot->SetPadding(FMargin(4.f, 0.f));
        }

        TabButtons.Add(Btn);
    }
}

void UMainMenuWidget::OnTabButtonClicked(int32 TabIndex)
{
    SwitchToTab(TabIndex);
}

void UMainMenuWidget::SwitchToTab(int32 TabIndex)
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
