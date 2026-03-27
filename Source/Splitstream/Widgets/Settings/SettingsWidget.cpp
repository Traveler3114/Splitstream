#include "SettingsWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Widgets/Settings/GraphicsWidget.h"
#include "Widgets/Settings/InputWidget.h"

void USettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind tab switches
    if (GraphicsTabButton)
    {
        GraphicsTabButton->OnClicked.RemoveDynamic(this, &USettingsWidget::OnGraphicsTabClicked);
        GraphicsTabButton->OnClicked.AddDynamic(this, &USettingsWidget::OnGraphicsTabClicked);
    }
    if (InputTabButton)
    {
        InputTabButton->OnClicked.RemoveDynamic(this, &USettingsWidget::OnInputTabClicked);
        InputTabButton->OnClicked.AddDynamic(this, &USettingsWidget::OnInputTabClicked);
    }

    // Apply/Back Buttons (if you expose them at top-level)
    if (ApplyButton)
    {
        ApplyButton->OnClicked.RemoveDynamic(this, &USettingsWidget::OnApplyClicked);
        ApplyButton->OnClicked.AddDynamic(this, &USettingsWidget::OnApplyClicked);
    }

    // Start on the Graphics tab by default
    if (SettingsSwitcher)
    {
        SettingsSwitcher->SetActiveWidgetIndex(0); // 0 = Graphics, 1 = Input
        UpdateTabHighlight();
    }
}

void USettingsWidget::OnGraphicsTabClicked()
{
    if (SettingsSwitcher)
    {
        SettingsSwitcher->SetActiveWidgetIndex(0);
        UpdateTabHighlight();
    }
}

void USettingsWidget::OnInputTabClicked()
{
    if (SettingsSwitcher)
    {
        SettingsSwitcher->SetActiveWidgetIndex(1);
        UpdateTabHighlight();
    }
}

void USettingsWidget::UpdateTabHighlight()
{
    if (!SettingsSwitcher) return;
    int32 Index = SettingsSwitcher->GetActiveWidgetIndex();

    // For label highlight (optional)
    if (GraphicsTabLabel)
    {
        GraphicsTabLabel->SetColorAndOpacity(Index == 0 ? FSlateColor(FLinearColor::Yellow) : FSlateColor(FLinearColor::White));
    }
    if (InputTabLabel)
    {
        InputTabLabel->SetColorAndOpacity(Index == 1 ? FSlateColor(FLinearColor::Yellow) : FSlateColor(FLinearColor::White));
    }
    // You can also change button images here if you want.
}

void USettingsWidget::OnApplyClicked()
{
    // Propagate apply to visible tab
    if (GraphicsWidget && SettingsSwitcher->GetActiveWidgetIndex() == 0)
        GraphicsWidget->ApplySettings();
    if (InputWidget && SettingsSwitcher->GetActiveWidgetIndex() == 1)
        InputWidget->ApplySettings();

    // You can close or notify success here if wanted
}

void USettingsWidget::OnBackClicked()
{
    SetVisibility(ESlateVisibility::Hidden);
}