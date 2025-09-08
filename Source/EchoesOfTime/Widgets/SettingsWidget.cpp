#include "SettingsWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "GameFramework/GameUserSettings.h"

void USettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // ----- Options -----
    ResolutionOptions = { FIntPoint(1280,720), FIntPoint(1920,1080), FIntPoint(2560,1440), FIntPoint(3840,2160) };
    ResolutionLabels = { TEXT("1280x720"), TEXT("1920x1080"), TEXT("2560x1440"), TEXT("3840x2160") };
    ResolutionIndex = 1; // 1920x1080 default

    // --- Render Scale ---
    RenderScaleMin = 0.25f;
    RenderScaleMax = 1.0f;
    RenderScale = 1.0f; // Default 100%

    ShadowsOptions = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic") };
    ShadowsIndex = 2;

    TexturesOptions = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic") };
    TexturesIndex = 2;

    AAOptions = { TEXT("Off"), TEXT("Low"), TEXT("High"), TEXT("Epic") };
    AAIndex = 2;

    PPOptions = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic") };
    PPIndex = 2;

    // ----- Bind buttons -----
    if (ApplyButton) {
        ApplyButton->OnClicked.RemoveDynamic(this, &USettingsWidget::OnApplyClicked);
        ApplyButton->OnClicked.AddDynamic(this, &USettingsWidget::OnApplyClicked);
    }
    if (BackButton) {
        BackButton->OnClicked.RemoveDynamic(this, &USettingsWidget::OnBackClicked);
        BackButton->OnClicked.AddDynamic(this, &USettingsWidget::OnBackClicked);
    }
    if (ResolutionLeftButton) {
        ResolutionLeftButton->OnClicked.RemoveDynamic(this, &USettingsWidget::OnResolutionLeft);
        ResolutionLeftButton->OnClicked.AddDynamic(this, &USettingsWidget::OnResolutionLeft);
    }
    if (ResolutionRightButton) {
        ResolutionRightButton->OnClicked.RemoveDynamic(this, &USettingsWidget::OnResolutionRight);
        ResolutionRightButton->OnClicked.AddDynamic(this, &USettingsWidget::OnResolutionRight);
    }

    // --- Render Scale Slider ---
    if (RenderScaleSlider)
    {
        RenderScaleSlider->SetMinValue(RenderScaleMin);
        RenderScaleSlider->SetMaxValue(RenderScaleMax);
        RenderScaleSlider->SetValue(RenderScale);

        RenderScaleSlider->OnValueChanged.RemoveDynamic(this, &USettingsWidget::OnRenderScaleChanged);
        RenderScaleSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnRenderScaleChanged);
    }

    if (ShadowsLeftButton) {
        ShadowsLeftButton->OnClicked.RemoveDynamic(this, &USettingsWidget::OnShadowsLeft);
        ShadowsLeftButton->OnClicked.AddDynamic(this, &USettingsWidget::OnShadowsLeft);
    }
    if (ShadowsRightButton) {
        ShadowsRightButton->OnClicked.RemoveDynamic(this, &USettingsWidget::OnShadowsRight);
        ShadowsRightButton->OnClicked.AddDynamic(this, &USettingsWidget::OnShadowsRight);
    }
    if (TexturesLeftButton) {
        TexturesLeftButton->OnClicked.RemoveDynamic(this, &USettingsWidget::OnTexturesLeft);
        TexturesLeftButton->OnClicked.AddDynamic(this, &USettingsWidget::OnTexturesLeft);
    }
    if (TexturesRightButton) {
        TexturesRightButton->OnClicked.RemoveDynamic(this, &USettingsWidget::OnTexturesRight);
        TexturesRightButton->OnClicked.AddDynamic(this, &USettingsWidget::OnTexturesRight);
    }
    if (AALeftButton) {
        AALeftButton->OnClicked.RemoveDynamic(this, &USettingsWidget::OnAALeft);
        AALeftButton->OnClicked.AddDynamic(this, &USettingsWidget::OnAALeft);
    }
    if (AARightButton) {
        AARightButton->OnClicked.RemoveDynamic(this, &USettingsWidget::OnAARight);
        AARightButton->OnClicked.AddDynamic(this, &USettingsWidget::OnAARight);
    }
    if (PPLeftButton) {
        PPLeftButton->OnClicked.RemoveDynamic(this, &USettingsWidget::OnPPLeft);
        PPLeftButton->OnClicked.AddDynamic(this, &USettingsWidget::OnPPLeft);
    }
    if (PPRightButton) {
        PPRightButton->OnClicked.RemoveDynamic(this, &USettingsWidget::OnPPRight);
        PPRightButton->OnClicked.AddDynamic(this, &USettingsWidget::OnPPRight);
    }

    UpdateTexts();
}

void USettingsWidget::UpdateTexts()
{
    if (ResolutionValueText)
        ResolutionValueText->SetText(FText::FromString(ResolutionLabels[ResolutionIndex]));

    // Render scale as percent
    if (RenderScaleValueText)
    {
        int32 Percent = FMath::RoundToInt(RenderScale * 100.0f);
        RenderScaleValueText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), Percent)));
    }

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

// --- Render Scale ---
void USettingsWidget::OnRenderScaleChanged(float Value)
{
    RenderScale = Value;
    UpdateTexts();
}

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

    // Render Scale (0.25f-1.0f)
    Settings->SetResolutionScaleNormalized(RenderScale);

    // Quality settings (0-3)
    Settings->SetShadowQuality(ShadowsIndex);
    Settings->SetTextureQuality(TexturesIndex);
    Settings->SetAntiAliasingQuality(AAIndex);
    Settings->SetPostProcessingQuality(PPIndex);

    Settings->ApplySettings(false);
}