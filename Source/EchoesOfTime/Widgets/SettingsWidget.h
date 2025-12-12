#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsWidget.generated.h"

class UTextBlock;
class UButton;
class USlider;
class UCheckBox;

UCLASS()
class ECHOESOFTIME_API USettingsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

protected:
    // Bindings to UMG widgets (must match variable names in designer)
    UPROPERTY(meta = (BindWidget))
    UButton* ApplyButton;

    UPROPERTY(meta = (BindWidget))
    UButton* BackButton;

    UPROPERTY(meta = (BindWidget))
    UButton* ResolutionLeftButton;

    UPROPERTY(meta = (BindWidget))
    UButton* ResolutionRightButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ResolutionValueText;

    // --- Window Mode ---
    UPROPERTY(meta = (BindWidget))
    UButton* WindowModeLeftButton;

    UPROPERTY(meta = (BindWidget))
    UButton* WindowModeRightButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* WindowModeValueText;

    // --- Render Scale ---
    UPROPERTY(meta = (BindWidget))
    USlider* RenderScaleSlider;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* RenderScaleValueText;

    // --- Shadows ---
    UPROPERTY(meta = (BindWidget))
    UButton* ShadowsLeftButton;

    UPROPERTY(meta = (BindWidget))
    UButton* ShadowsRightButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ShadowsValueText;

    // --- Textures ---
    UPROPERTY(meta = (BindWidget))
    UButton* TexturesLeftButton;

    UPROPERTY(meta = (BindWidget))
    UButton* TexturesRightButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TexturesValueText;

    // --- Anti-Aliasing ---
    UPROPERTY(meta = (BindWidget))
    UButton* AALeftButton;

    UPROPERTY(meta = (BindWidget))
    UButton* AARightButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* AAValueText;

    // --- Post Processing ---
    UPROPERTY(meta = (BindWidget))
    UButton* PPLeftButton;

    UPROPERTY(meta = (BindWidget))
    UButton* PPRightButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* PPValueText;

    // --- VSync ---
    UPROPERTY(meta = (BindWidget))
    UCheckBox* VSyncCheckBox;

    // --- FPS Limit ---
    UPROPERTY(meta = (BindWidget))
    USlider* FPSLimitSlider;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* FPSLimitValueText;

    // Option data
    int32 ResolutionIndex;
    TArray<FIntPoint> ResolutionOptions;
    TArray<FString> ResolutionLabels;

    // --- Window Mode ---
    int32 WindowModeIndex;
    TArray<EWindowMode::Type> WindowModeOptions;
    TArray<FString> WindowModeLabels;

    // --- Render Scale ---
    float RenderScale; // 0.25f - 1.0f
    float RenderScaleMin;
    float RenderScaleMax;

    // --- Quality ---
    int32 ShadowsIndex;
    TArray<FString> ShadowsOptions;

    int32 TexturesIndex;
    TArray<FString> TexturesOptions;

    int32 AAIndex;
    TArray<FString> AAOptions;

    int32 PPIndex;
    TArray<FString> PPOptions;

    // --- VSync / FPS Limit Data ---
    bool bVSyncEnabled;

    // FPS limit in frames per second
    float FPSLimit;
    float FPSMin;
    float FPSMax;      // Slider max; if slider == FPSMax => unlimited
    bool bFPSUnlimited;

    // Handlers
    UFUNCTION()
    void OnApplyClicked();

    UFUNCTION()
    void OnBackClicked();

    UFUNCTION()
    void OnResolutionLeft();

    UFUNCTION()
    void OnResolutionRight();

    // --- Window Mode ---
    UFUNCTION()
    void OnWindowModeLeft();

    UFUNCTION()
    void OnWindowModeRight();

    // --- Render Scale ---
    UFUNCTION()
    void OnRenderScaleChanged(float Value);

    // --- Quality ---
    UFUNCTION()
    void OnShadowsLeft();

    UFUNCTION()
    void OnShadowsRight();

    UFUNCTION()
    void OnTexturesLeft();

    UFUNCTION()
    void OnTexturesRight();

    UFUNCTION()
    void OnAALeft();

    UFUNCTION()
    void OnAARight();

    UFUNCTION()
    void OnPPLeft();

    UFUNCTION()
    void OnPPRight();

    // --- VSync / FPS ---
    UFUNCTION()
    void OnVSyncChanged(bool bIsChecked);

    UFUNCTION()
    void OnFPSLimitChanged(float Value);

    void UpdateTexts();
    void ApplySettings();
};