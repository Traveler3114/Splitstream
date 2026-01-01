#include "GraphicsWidget.h"
#include "EnumOptionWidget.h"
#include "SliderWidget.h"
#include "Components/CheckBox.h"
#include "GameFramework/GameUserSettings.h"
#include "HAL/IConsoleManager.h"

void UGraphicsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // --- Option Data ---
    ResolutionOptions = { FIntPoint(1280,720), FIntPoint(1920,1080), FIntPoint(2560,1440), FIntPoint(3840,2160) };
    ResolutionLabels = { TEXT("1280x720"), TEXT("1920x1080"), TEXT("2560x1440"), TEXT("3840x2160") };
    ResolutionIndex = 1;

    WindowModeOptions = { EWindowMode::Fullscreen, EWindowMode::WindowedFullscreen, EWindowMode::Windowed };
    WindowModeLabels = { TEXT("Fullscreen"), TEXT("Borderless"), TEXT("Windowed") };
    WindowModeIndex = 0;

    RenderScaleMin = 0.25f;
    RenderScaleMax = 1.0f;
    RenderScale = RenderScaleMax;

    ShadowsOptions = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic") };
    ShadowsIndex = 2;
    TexturesOptions = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic") };
    TexturesIndex = 2;
    AAOptions = { TEXT("Off"), TEXT("Low"), TEXT("High"), TEXT("Epic") };
    AAIndex = 2;
    PPOptions = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic") };
    PPIndex = 2;

    FPSMin = 30.0f;
    FPSMax = 240.0f;
    FPSLimit = FPSMax;
    bFPSUnlimited = true;
    bVSyncEnabled = false;

    // --- Load Settings from GameUserSettings ---
    UGameUserSettings* Settings = GEngine->GetGameUserSettings();
    if (Settings)
    {
        Settings->LoadSettings(false);

        FIntPoint CurrentRes = Settings->GetScreenResolution();
        for (int32 i = 0; i < ResolutionOptions.Num(); ++i)
            if (ResolutionOptions[i] == CurrentRes) { ResolutionIndex = i; break; }

        WindowModeIndex = WindowModeOptions.IndexOfByKey(Settings->GetFullscreenMode());

        RenderScale = FMath::Clamp(Settings->GetResolutionScaleNormalized(), RenderScaleMin, RenderScaleMax);
        if (RenderScale <= 0.0f) RenderScale = RenderScaleMax;

        ShadowsIndex = FMath::Clamp(Settings->GetShadowQuality(), 0, ShadowsOptions.Num() - 1);
        TexturesIndex = FMath::Clamp(Settings->GetTextureQuality(), 0, TexturesOptions.Num() - 1);
        AAIndex = FMath::Clamp(Settings->GetAntiAliasingQuality(), 0, AAOptions.Num() - 1);
        PPIndex = FMath::Clamp(Settings->GetPostProcessingQuality(), 0, PPOptions.Num() - 1);

        bVSyncEnabled = Settings->IsVSyncEnabled();

        IConsoleVariable* MaxFPSVar = IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS"));
        float CurrentMaxFPS = MaxFPSVar ? MaxFPSVar->GetFloat() : 0.0f;
        if (CurrentMaxFPS <= 0.0f)
        {
            bFPSUnlimited = true;
            FPSLimit = FPSMax;
        }
        else
        {
            bFPSUnlimited = false;
            FPSLimit = FMath::Clamp(CurrentMaxFPS, FPSMin, FPSMax);
        }
    }

    SetupWidgets();
}

void UGraphicsWidget::SetupWidgets()
{
    // For each delegate add, remove it first!
    if (ResolutionWidget)
    {
        ResolutionWidget->Setup(FText::FromString(TEXT("Resolution")), ResolutionLabels, ResolutionIndex);
        ResolutionWidget->OnEnumChanged.RemoveDynamic(this, &UGraphicsWidget::OnResolutionChanged); // <-- ADD THIS!
        ResolutionWidget->OnEnumChanged.AddDynamic(this, &UGraphicsWidget::OnResolutionChanged);
    }
    if (WindowModeWidget)
    {
        WindowModeWidget->Setup(FText::FromString(TEXT("Window Mode")), WindowModeLabels, WindowModeIndex);
        WindowModeWidget->OnEnumChanged.RemoveDynamic(this, &UGraphicsWidget::OnWindowModeChanged);
        WindowModeWidget->OnEnumChanged.AddDynamic(this, &UGraphicsWidget::OnWindowModeChanged);
    }
    if (RenderScaleWidget)
    {
        RenderScaleWidget->Setup(FText::FromString(TEXT("Render Scale")), RenderScaleMin, RenderScaleMax, RenderScale, 2);
        RenderScaleWidget->OnValueChanged.RemoveDynamic(this, &UGraphicsWidget::OnRenderScaleChanged);
        RenderScaleWidget->OnValueChanged.AddDynamic(this, &UGraphicsWidget::OnRenderScaleChanged);
    }
    if (ShadowsWidget)
    {
        ShadowsWidget->Setup(FText::FromString(TEXT("Shadows Quality")), ShadowsOptions, ShadowsIndex);
        ShadowsWidget->OnEnumChanged.RemoveDynamic(this, &UGraphicsWidget::OnShadowsChanged);
        ShadowsWidget->OnEnumChanged.AddDynamic(this, &UGraphicsWidget::OnShadowsChanged);
    }
    if (TexturesWidget)
    {
        TexturesWidget->Setup(FText::FromString(TEXT("Textures Quality")), TexturesOptions, TexturesIndex);
        TexturesWidget->OnEnumChanged.RemoveDynamic(this, &UGraphicsWidget::OnTexturesChanged);
        TexturesWidget->OnEnumChanged.AddDynamic(this, &UGraphicsWidget::OnTexturesChanged);
    }
    if (AAWidget)
    {
        AAWidget->Setup(FText::FromString(TEXT("AntiAliasing")), AAOptions, AAIndex);
        AAWidget->OnEnumChanged.RemoveDynamic(this, &UGraphicsWidget::OnAAChanged);
        AAWidget->OnEnumChanged.AddDynamic(this, &UGraphicsWidget::OnAAChanged);
    }
    if (PPWidget)
    {
        PPWidget->Setup(FText::FromString(TEXT("Post Process")), PPOptions, PPIndex);
        PPWidget->OnEnumChanged.RemoveDynamic(this, &UGraphicsWidget::OnPPChanged);
        PPWidget->OnEnumChanged.AddDynamic(this, &UGraphicsWidget::OnPPChanged);
    }
    if (FPSLimitWidget)
    {
        FPSLimitWidget->Setup(FText::FromString(TEXT("Frame Rate Limit")), FPSMin, FPSMax, FPSLimit, 0);
        FPSLimitWidget->OnValueChanged.RemoveDynamic(this, &UGraphicsWidget::OnFPSLimitChanged);
        FPSLimitWidget->OnValueChanged.AddDynamic(this, &UGraphicsWidget::OnFPSLimitChanged);
    }
    if (VSyncCheckBox)
    {
        VSyncCheckBox->OnCheckStateChanged.RemoveAll(this);
        VSyncCheckBox->OnCheckStateChanged.AddDynamic(this, &UGraphicsWidget::OnVSyncChanged);
        VSyncCheckBox->SetIsChecked(bVSyncEnabled);
    }
}

void UGraphicsWidget::OnResolutionChanged(int32 Index) { ResolutionIndex = Index; }
void UGraphicsWidget::OnWindowModeChanged(int32 Index) { WindowModeIndex = Index; }
void UGraphicsWidget::OnRenderScaleChanged(float Value) { RenderScale = Value; }
void UGraphicsWidget::OnShadowsChanged(int32 Index) { ShadowsIndex = Index; }
void UGraphicsWidget::OnTexturesChanged(int32 Index) { TexturesIndex = Index; }
void UGraphicsWidget::OnAAChanged(int32 Index) { AAIndex = Index; }
void UGraphicsWidget::OnPPChanged(int32 Index) { PPIndex = Index; }
void UGraphicsWidget::OnVSyncChanged(bool bChecked) { bVSyncEnabled = bChecked; }
void UGraphicsWidget::OnFPSLimitChanged(float Value)
{
    if (FMath::Abs(Value - FPSMax) < KINDA_SMALL_NUMBER)
    {
        bFPSUnlimited = true;
        FPSLimit = FPSMax;
    }
    else
    {
        bFPSUnlimited = false;
        FPSLimit = FMath::Clamp(Value, FPSMin, FPSMax);
    }
}

void UGraphicsWidget::ApplySettings()
{
    UGameUserSettings* Settings = GEngine->GetGameUserSettings();
    if (!Settings) return;
    Settings->SetScreenResolution(ResolutionOptions[ResolutionIndex]);
    Settings->SetFullscreenMode(WindowModeOptions[WindowModeIndex]);
    Settings->SetResolutionScaleNormalized(RenderScale);
    Settings->SetShadowQuality(ShadowsIndex);
    Settings->SetTextureQuality(TexturesIndex);
    Settings->SetAntiAliasingQuality(AAIndex);
    Settings->SetPostProcessingQuality(PPIndex);
    Settings->SetVSyncEnabled(bVSyncEnabled);

    IConsoleVariable* MaxFPSVar = IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS"));
    if (MaxFPSVar)
    {
        if (bVSyncEnabled || bFPSUnlimited)
            MaxFPSVar->Set(0.0f);
        else
            MaxFPSVar->Set(FMath::Clamp(FPSLimit, FPSMin, FPSMax));
    }

    Settings->SaveSettings();
    Settings->ApplySettings(false);
}