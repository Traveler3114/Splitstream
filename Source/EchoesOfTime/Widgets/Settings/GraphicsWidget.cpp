#include "GraphicsWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "GameFramework/GameUserSettings.h"
#include "Misc/EnumRange.h"
#include "HAL/IConsoleManager.h"   // for console variable t.MaxFPS

void UGraphicsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // ----- Options -----
    ResolutionOptions = { FIntPoint(1280,720), FIntPoint(1920,1080), FIntPoint(2560,1440), FIntPoint(3840,2160) };
    ResolutionLabels = { TEXT("1280x720"), TEXT("1920x1080"), TEXT("2560x1440"), TEXT("3840x2160") };
    ResolutionIndex = 1; // Default

    WindowModeOptions = { EWindowMode::Fullscreen, EWindowMode::WindowedFullscreen, EWindowMode::Windowed };
    WindowModeLabels = { TEXT("Fullscreen"), TEXT("Borderless"), TEXT("Windowed") };
    WindowModeIndex = 0;

    RenderScaleMin = 0.25f;
    RenderScaleMax = 1.0f;
    RenderScale = RenderScaleMax; // default to 1.0

    ShadowsOptions = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic") };
    ShadowsIndex = 2;

    TexturesOptions = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic") };
    TexturesIndex = 2;

    AAOptions = { TEXT("Off"), TEXT("Low"), TEXT("High"), TEXT("Epic") };
    AAIndex = 2;

    PPOptions = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic") };
    PPIndex = 2;

    // --- FPS/VSync defaults ---
    FPSMin = 30.0f;
    FPSMax = 240.0f;   // top of slider = "unlimited"
    FPSLimit = FPSMax;
    bFPSUnlimited = true;
    bVSyncEnabled = false;

    // ---- Load settings from last session ----
    UGameUserSettings* Settings = GEngine->GetGameUserSettings();
    if (Settings)
    {
        Settings->LoadSettings(false);

        // Resolution
        FIntPoint CurrentRes = Settings->GetScreenResolution();
        for (int32 i = 0; i < ResolutionOptions.Num(); ++i)
        {
            if (ResolutionOptions[i] == CurrentRes)
            {
                ResolutionIndex = i;
                break;
            }
        }

        // Window Mode
        WindowModeIndex = WindowModeOptions.IndexOfByKey(Settings->GetFullscreenMode());

        // Render Scale (clamped + fallback)
        RenderScale = FMath::Clamp(
            Settings->GetResolutionScaleNormalized(),
            RenderScaleMin,
            RenderScaleMax
        );
        if (RenderScale <= 0.0f)
        {
            RenderScale = RenderScaleMax; // 1.0f
        }

        // Quality settings
        ShadowsIndex = FMath::Clamp(Settings->GetShadowQuality(), 0, ShadowsOptions.Num() - 1);
        TexturesIndex = FMath::Clamp(Settings->GetTextureQuality(), 0, TexturesOptions.Num() - 1);
        AAIndex = FMath::Clamp(Settings->GetAntiAliasingQuality(), 0, AAOptions.Num() - 1);
        PPIndex = FMath::Clamp(Settings->GetPostProcessingQuality(), 0, PPOptions.Num() - 1);

        // --- VSync / FPS from settings ---
        // VSync (stored by GameUserSettings)
        bVSyncEnabled = Settings->IsVSyncEnabled();

        // FPS limit from console variable t.MaxFPS
        // FPS limit from console variable t.MaxFPS
        {
            IConsoleVariable* MaxFPSVar = IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS"));
            float CurrentMaxFPS = MaxFPSVar ? MaxFPSVar->GetFloat() : 0.0f;

            // If the game has no limit (0 or negative): treat as Unlimited.
            if (CurrentMaxFPS <= 0.0f)
            {
                bFPSUnlimited = true;
                FPSLimit = FPSMax;  // slider to max for "Unlimited"
            }
            else
            {
                bFPSUnlimited = false;
                FPSLimit = FMath::Clamp(CurrentMaxFPS, FPSMin, FPSMax);
            }
        }
    }

    if (ResolutionLeftButton) {
        ResolutionLeftButton->OnClicked.RemoveDynamic(this, &UGraphicsWidget::OnResolutionLeft);
        ResolutionLeftButton->OnClicked.AddDynamic(this, &UGraphicsWidget::OnResolutionLeft);
    }
    if (ResolutionRightButton) {
        ResolutionRightButton->OnClicked.RemoveDynamic(this, &UGraphicsWidget::OnResolutionRight);
        ResolutionRightButton->OnClicked.AddDynamic(this, &UGraphicsWidget::OnResolutionRight);
    }
    if (WindowModeLeftButton) {
        WindowModeLeftButton->OnClicked.RemoveDynamic(this, &UGraphicsWidget::OnWindowModeLeft);
        WindowModeLeftButton->OnClicked.AddDynamic(this, &UGraphicsWidget::OnWindowModeLeft);
    }
    if (WindowModeRightButton) {
        WindowModeRightButton->OnClicked.RemoveDynamic(this, &UGraphicsWidget::OnWindowModeRight);
        WindowModeRightButton->OnClicked.AddDynamic(this, &UGraphicsWidget::OnWindowModeRight);
    }
    if (RenderScaleSlider)
    {
        RenderScaleSlider->SetMinValue(RenderScaleMin);
        RenderScaleSlider->SetMaxValue(RenderScaleMax);
        RenderScaleSlider->SetValue(RenderScale);

        RenderScaleSlider->OnValueChanged.RemoveDynamic(this, &UGraphicsWidget::OnRenderScaleChanged);
        RenderScaleSlider->OnValueChanged.AddDynamic(this, &UGraphicsWidget::OnRenderScaleChanged);
    }
    if (ShadowsLeftButton) {
        ShadowsLeftButton->OnClicked.RemoveDynamic(this, &UGraphicsWidget::OnShadowsLeft);
        ShadowsLeftButton->OnClicked.AddDynamic(this, &UGraphicsWidget::OnShadowsLeft);
    }
    if (ShadowsRightButton) {
        ShadowsRightButton->OnClicked.RemoveDynamic(this, &UGraphicsWidget::OnShadowsRight);
        ShadowsRightButton->OnClicked.AddDynamic(this, &UGraphicsWidget::OnShadowsRight);
    }
    if (TexturesLeftButton) {
        TexturesLeftButton->OnClicked.RemoveDynamic(this, &UGraphicsWidget::OnTexturesLeft);
        TexturesLeftButton->OnClicked.AddDynamic(this, &UGraphicsWidget::OnTexturesLeft);
    }
    if (TexturesRightButton) {
        TexturesRightButton->OnClicked.RemoveDynamic(this, &UGraphicsWidget::OnTexturesRight);
        TexturesRightButton->OnClicked.AddDynamic(this, &UGraphicsWidget::OnTexturesRight);
    }
    if (AALeftButton) {
        AALeftButton->OnClicked.RemoveDynamic(this, &UGraphicsWidget::OnAALeft);
        AALeftButton->OnClicked.AddDynamic(this, &UGraphicsWidget::OnAALeft);
    }
    if (AARightButton) {
        AARightButton->OnClicked.RemoveDynamic(this, &UGraphicsWidget::OnAARight);
        AARightButton->OnClicked.AddDynamic(this, &UGraphicsWidget::OnAARight);
    }
    if (PPLeftButton) {
        PPLeftButton->OnClicked.RemoveDynamic(this, &UGraphicsWidget::OnPPLeft);
        PPLeftButton->OnClicked.AddDynamic(this, &UGraphicsWidget::OnPPLeft);
    }
    if (PPRightButton) {
        PPRightButton->OnClicked.RemoveDynamic(this, &UGraphicsWidget::OnPPRight);
        PPRightButton->OnClicked.AddDynamic(this, &UGraphicsWidget::OnPPRight);
    }

    // --- VSync ---
    if (VSyncCheckBox)
    {
        VSyncCheckBox->OnCheckStateChanged.RemoveDynamic(this, &UGraphicsWidget::OnVSyncChanged);
        VSyncCheckBox->OnCheckStateChanged.AddDynamic(this, &UGraphicsWidget::OnVSyncChanged);
        VSyncCheckBox->SetIsChecked(bVSyncEnabled);
    }

    // --- FPS Limit ---
    if (FPSLimitSlider)
    {
        FPSLimitSlider->SetMinValue(FPSMin);
        FPSLimitSlider->SetMaxValue(FPSMax);

        // If unlimited, put slider to max
        float SliderValue = bFPSUnlimited ? FPSMax : FPSLimit;
        FPSLimitSlider->SetValue(SliderValue);

        FPSLimitSlider->OnValueChanged.RemoveDynamic(this, &UGraphicsWidget::OnFPSLimitChanged);
        FPSLimitSlider->OnValueChanged.AddDynamic(this, &UGraphicsWidget::OnFPSLimitChanged);
    }

    UpdateTexts();
}

void UGraphicsWidget::UpdateTexts()
{
    if (ResolutionValueText)
        ResolutionValueText->SetText(FText::FromString(ResolutionLabels[ResolutionIndex]));

    // Window Mode
    if (WindowModeValueText)
        WindowModeValueText->SetText(FText::FromString(WindowModeLabels[WindowModeIndex]));

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

    // --- FPS Limit text ---
    if (FPSLimitValueText)
    {
        if (bFPSUnlimited)
        {
            FPSLimitValueText->SetText(FText::FromString(TEXT("Unlimited")));
        }
        else
        {
            int32 FPSInt = FMath::RoundToInt(FPSLimit);
            FPSLimitValueText->SetText(FText::FromString(FString::Printf(TEXT("%d FPS"), FPSInt)));
        }
    }
}

void UGraphicsWidget::OnResolutionLeft()
{
    ResolutionIndex = (ResolutionIndex - 1 + ResolutionOptions.Num()) % ResolutionOptions.Num();
    UpdateTexts();
}

void UGraphicsWidget::OnResolutionRight()
{
    ResolutionIndex = (ResolutionIndex + 1) % ResolutionOptions.Num();
    UpdateTexts();
}

// --- Window Mode ---
void UGraphicsWidget::OnWindowModeLeft()
{
    WindowModeIndex = (WindowModeIndex - 1 + WindowModeOptions.Num()) % WindowModeOptions.Num();
    UpdateTexts();
}

void UGraphicsWidget::OnWindowModeRight()
{
    WindowModeIndex = (WindowModeIndex + 1) % WindowModeOptions.Num();
    UpdateTexts();
}

// --- Render Scale ---
void UGraphicsWidget::OnRenderScaleChanged(float Value)
{
    RenderScale = Value;
    UpdateTexts();
}

// --- Quality ---
void UGraphicsWidget::OnShadowsLeft()
{
    ShadowsIndex = (ShadowsIndex - 1 + ShadowsOptions.Num()) % ShadowsOptions.Num();
    UpdateTexts();
}

void UGraphicsWidget::OnShadowsRight()
{
    ShadowsIndex = (ShadowsIndex + 1) % ShadowsOptions.Num();
    UpdateTexts();
}

void UGraphicsWidget::OnTexturesLeft()
{
    TexturesIndex = (TexturesIndex - 1 + TexturesOptions.Num()) % TexturesOptions.Num();
    UpdateTexts();
}

void UGraphicsWidget::OnTexturesRight()
{
    TexturesIndex = (TexturesIndex + 1) % TexturesOptions.Num();
    UpdateTexts();
}

void UGraphicsWidget::OnAALeft()
{
    AAIndex = (AAIndex - 1 + AAOptions.Num()) % AAOptions.Num();
    UpdateTexts();
}

void UGraphicsWidget::OnAARight()
{
    AAIndex = (AAIndex + 1) % AAOptions.Num();
    UpdateTexts();
}

void UGraphicsWidget::OnPPLeft()
{
    PPIndex = (PPIndex - 1 + PPOptions.Num()) % PPOptions.Num();
    UpdateTexts();
}

void UGraphicsWidget::OnPPRight()
{
    PPIndex = (PPIndex + 1) % PPOptions.Num();
    UpdateTexts();
}

// --- VSync / FPS ---
void UGraphicsWidget::OnVSyncChanged(bool bIsChecked)
{
    bVSyncEnabled = bIsChecked;
    UpdateTexts();
}

void UGraphicsWidget::OnFPSLimitChanged(float Value)
{
    // Value will be between FPSMin and FPSMax
    if (Value >= FPSMax - KINDA_SMALL_NUMBER)
    {
        bFPSUnlimited = true;
        FPSLimit = FPSMax;
    }
    else
    {
        bFPSUnlimited = false;
        FPSLimit = FMath::Clamp(Value, FPSMin, FPSMax);
    }

    UpdateTexts();
}


void UGraphicsWidget::ApplySettings()
{
    UGameUserSettings* Settings = GEngine->GetGameUserSettings();
    if (!Settings) return;

    // Resolution
    Settings->SetScreenResolution(ResolutionOptions[ResolutionIndex]);

    // Window Mode
    Settings->SetFullscreenMode(WindowModeOptions[WindowModeIndex]);

    // Render Scale (0.25f-1.0f)
    Settings->SetResolutionScaleNormalized(RenderScale);

    // Quality settings (0-3)
    Settings->SetShadowQuality(ShadowsIndex);
    Settings->SetTextureQuality(TexturesIndex);
    Settings->SetAntiAliasingQuality(AAIndex);
    Settings->SetPostProcessingQuality(PPIndex);

    // --- VSync ---
    Settings->SetVSyncEnabled(bVSyncEnabled);

    // --- FPS Limit with VSync override ---
    {
        IConsoleVariable* MaxFPSVar = IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS"));
        if (MaxFPSVar)
        {
            if (bVSyncEnabled)
            {
                // VSync overrides FPS limiter; let engine run free,
                // monitor refresh + vsync will be the real cap.
                MaxFPSVar->Set(0.0f); // 0 => unlimited
            }
            else
            {
                if (bFPSUnlimited)
                {
                    MaxFPSVar->Set(0.0f); // unlimited
                }
                else
                {
                    float ClampedFPS = FMath::Clamp(FPSLimit, FPSMin, FPSMax);
                    MaxFPSVar->Set(ClampedFPS);
                }
            }
        }
    }

    // Save and apply
    Settings->SaveSettings();
    Settings->ApplySettings(false);
}