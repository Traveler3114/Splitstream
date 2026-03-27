#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/ISettingsTabInterface.h"
#include "GraphicsWidget.generated.h"

class UCheckBox;
class UEnumOptionWidget;
class USliderWidget;

UCLASS()
class SPLITSTREAM_API UGraphicsWidget : public UUserWidget, public ISettingsTabInterface
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    // ISettingsTabInterface
    virtual void ApplySettings_Implementation() override;

protected:
    UPROPERTY(meta = (BindWidget))
    UEnumOptionWidget* ResolutionWidget;
    UPROPERTY(meta = (BindWidget))
    UEnumOptionWidget* WindowModeWidget;
    UPROPERTY(meta = (BindWidget))
    USliderWidget* RenderScaleWidget;
    UPROPERTY(meta = (BindWidget))
    UEnumOptionWidget* ShadowsWidget;
    UPROPERTY(meta = (BindWidget))
    UEnumOptionWidget* TexturesWidget;
    UPROPERTY(meta = (BindWidget))
    UEnumOptionWidget* AAWidget;
    UPROPERTY(meta = (BindWidget))
    UEnumOptionWidget* PPWidget;
    UPROPERTY(meta = (BindWidget))
    USliderWidget* FPSLimitWidget;
    UPROPERTY(meta = (BindWidget))
    UCheckBox* VSyncCheckBox;

    int32 ResolutionIndex;
    TArray<FIntPoint> ResolutionOptions;
    TArray<FString> ResolutionLabels;
    int32 WindowModeIndex;
    TArray<EWindowMode::Type> WindowModeOptions;
    TArray<FString> WindowModeLabels;
    float RenderScale;
    float RenderScaleMin;
    float RenderScaleMax;
    int32 ShadowsIndex;
    TArray<FString> ShadowsOptions;
    int32 TexturesIndex;
    TArray<FString> TexturesOptions;
    int32 AAIndex;
    TArray<FString> AAOptions;
    int32 PPIndex;
    TArray<FString> PPOptions;
    bool bVSyncEnabled;
    float FPSLimit;
    float FPSMin;
    float FPSMax;
    bool bFPSUnlimited;

    void SetupWidgets();

    UFUNCTION() void OnResolutionChanged(int32 Index);
    UFUNCTION() void OnWindowModeChanged(int32 Index);
    UFUNCTION() void OnRenderScaleChanged(float Value);
    UFUNCTION() void OnShadowsChanged(int32 Index);
    UFUNCTION() void OnTexturesChanged(int32 Index);
    UFUNCTION() void OnAAChanged(int32 Index);
    UFUNCTION() void OnPPChanged(int32 Index);
    UFUNCTION() void OnVSyncChanged(bool bChecked);
    UFUNCTION() void OnFPSLimitChanged(float Value);
};
