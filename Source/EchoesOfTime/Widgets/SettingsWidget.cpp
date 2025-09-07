#include "SettingsWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameUserSettings.h"

void USettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // ----- Options -----
    ResolutionOptions = { FIntPoint(1280,720), FIntPoint(1920,1080), FIntPoint(2560,1440), FIntPoint(3840,2160) };
    ResolutionLabels = { TEXT("1280x720"), TEXT("1920x1080"), TEXT("2560x1440"), TEXT("3840x2160") };
    ResolutionIndex = 1; // 1920x1080 default

    ShadowsOptions = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic") };
    ShadowsIndex = 2;

    TexturesOptions = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic") };
    TexturesIndex = 2;

    AAOptions = { TEXT("Off"), TEXT("Low"), TEXT("High"), TEXT("Epic") };
    AAIndex = 2;

    PPOptions = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic") };
    PPIndex = 2;

    // ----- Bind buttons -----
    if (ApplyButton)
        ApplyButton->OnClicked.AddDynamic(this, &USettingsWidget::OnApplyClicked);
    if (BackButton)
        BackButton->OnClicked.AddDynamic(this, &USettingsWidget::OnBackClicked);

    if (ResolutionLeftButton)
        ResolutionLeftButton->OnClicked.AddDynamic(this, &USettingsWidget::OnResolutionLeft);
    if (ResolutionRightButton)
        ResolutionRightButton->OnClicked.AddDynamic(this, &USettingsWidget::OnResolutionRight);

    if (ShadowsLeftButton)
        ShadowsLeftButton->OnClicked.AddDynamic(this, &USettingsWidget::OnShadowsLeft);
    if (ShadowsRightButton)
        ShadowsRightButton->OnClicked.AddDynamic(this, &USettingsWidget::OnShadowsRight);

    if (TexturesLeftButton)
        TexturesLeftButton->OnClicked.AddDynamic(this, &USettingsWidget::OnTexturesLeft);
    if (TexturesRightButton)
        TexturesRightButton->OnClicked.AddDynamic(this, &USettingsWidget::OnTexturesRight);

    if (AALeftButton)
        AALeftButton->OnClicked.AddDynamic(this, &USettingsWidget::OnAALeft);
    if (AARightButton)
        AARightButton->OnClicked.AddDynamic(this, &USettingsWidget::OnAARight);

    if (PPLeftButton)
        PPLeftButton->OnClicked.AddDynamic(this, &USettingsWidget::OnPPLeft);
    if (PPRightButton)
        PPRightButton->OnClicked.AddDynamic(this, &USettingsWidget::OnPPRight);

    UpdateTexts();
}

void USettingsWidget::UpdateTexts()
{
    if (ResolutionValueText)
        ResolutionValueText->SetText(FText::FromString(ResolutionLabels[ResolutionIndex]));
    if (ShadowsValueText)
        ShadowsValueText->SetText(FText::FromString(ShadowsOptions[ShadowsIndex]));
    if (TexturesValueText)
        TexturesValueText->SetText(FText::FromString(TexturesOptions[TexturesIndex]));
    if (AAValueText)
        AAValueText->SetText(FText::FromString(AAOptions[AAIndex]));
    if (PPValueText)
        PPValueText->SetText(FText::FromString(PPOptions[PPIndex]));
}

void USettingsWidget::OnResolutionLeft() { ResolutionIndex = (ResolutionIndex - 1 + ResolutionOptions.Num()) % ResolutionOptions.Num(); UpdateTexts(); }
void USettingsWidget::OnResolutionRight() { ResolutionIndex = (ResolutionIndex + 1) % ResolutionOptions.Num(); UpdateTexts(); }
void USettingsWidget::OnShadowsLeft() { ShadowsIndex = (ShadowsIndex - 1 + ShadowsOptions.Num()) % ShadowsOptions.Num(); UpdateTexts(); }
void USettingsWidget::OnShadowsRight() { ShadowsIndex = (ShadowsIndex + 1) % ShadowsOptions.Num(); UpdateTexts(); }
void USettingsWidget::OnTexturesLeft() { TexturesIndex = (TexturesIndex - 1 + TexturesOptions.Num()) % TexturesOptions.Num(); UpdateTexts(); }
void USettingsWidget::OnTexturesRight() { TexturesIndex = (TexturesIndex + 1) % TexturesOptions.Num(); UpdateTexts(); }
void USettingsWidget::OnAALeft() { AAIndex = (AAIndex - 1 + AAOptions.Num()) % AAOptions.Num(); UpdateTexts(); }
void USettingsWidget::OnAARight() { AAIndex = (AAIndex + 1) % AAOptions.Num(); UpdateTexts(); }
void USettingsWidget::OnPPLeft() { PPIndex = (PPIndex - 1 + PPOptions.Num()) % PPOptions.Num(); UpdateTexts(); }
void USettingsWidget::OnPPRight() { PPIndex = (PPIndex + 1) % PPOptions.Num(); UpdateTexts(); }

void USettingsWidget::OnApplyClicked()
{
    ApplySettings();
}

void USettingsWidget::OnBackClicked()
{
    SetVisibility(ESlateVisibility::Hidden);
}

void USettingsWidget::ApplySettings()
{
    UGameUserSettings* Settings = GEngine->GetGameUserSettings();
    if (!Settings) return;

    // Resolution
    Settings->SetScreenResolution(ResolutionOptions[ResolutionIndex]);

    // Quality settings (0-3)
    Settings->SetShadowQuality(ShadowsIndex);
    Settings->SetTextureQuality(TexturesIndex);
    Settings->SetAntiAliasingQuality(AAIndex);
    Settings->SetPostProcessingQuality(PPIndex);

    Settings->ApplySettings(false);
}