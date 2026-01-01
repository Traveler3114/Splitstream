#include "PauseMenuWidget.h"
#include "Components/Button.h"
#include "Widgets/Settings/SettingsWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Controllers/DefaultPlayerController.h"
#include "Kismet/KismetSystemLibrary.h"

void UPauseMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (resume_btn) {
        resume_btn->OnClicked.RemoveDynamic(this, &UPauseMenuWidget::OnResumeClicked);
        resume_btn->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResumeClicked);
    }

    if (settings_btn) {
        settings_btn->OnClicked.RemoveDynamic(this, &UPauseMenuWidget::OnSettingsClicked);
        settings_btn->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnSettingsClicked);
    }

    if (mainmenu_btn) {
        mainmenu_btn->OnClicked.RemoveDynamic(this, &UPauseMenuWidget::OnMainMenuClicked);
        mainmenu_btn->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnMainMenuClicked);
    }

    if (quit_btn) {
        quit_btn->OnClicked.RemoveDynamic(this, &UPauseMenuWidget::OnQuitClicked);
        quit_btn->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnQuitClicked);
    }

    // Hide settings widget initially (if it exists in the widget tree)
    if (SettingsWidget)
        SettingsWidget->SetVisibility(ESlateVisibility::Hidden);
}

void UPauseMenuWidget::OnResumeClicked()
{
    RemoveFromParent();
    OnPauseMenuResumed.Broadcast(); // <-- Broadcast delegate!
}

void UPauseMenuWidget::OnSettingsClicked()
{
    if (SettingsWidget)
        SettingsWidget->SetVisibility(ESlateVisibility::Visible);
}

void UPauseMenuWidget::OnMainMenuClicked()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ADefaultPlayerController* MyPC = Cast<ADefaultPlayerController>(PC))
        {
            MyPC->RequestLeaveToMainMenu();
        }
    }
}

void UPauseMenuWidget::OnQuitClicked()
{
    UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}