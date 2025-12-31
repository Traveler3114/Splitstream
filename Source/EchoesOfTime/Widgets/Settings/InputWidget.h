#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputWidget.generated.h"

class UTextBlock;
class USlider;

UCLASS()
class ECHOESOFTIME_API UInputWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    void ApplySettings();

protected:
    // --- Mouse Sensitivity ---
    UPROPERTY(meta = (BindWidget))
    USlider* MouseSensitivitySlider;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* MouseSensitivityValueText;

    float MouseSensitivity;
    float MouseSensitivityMin;
    float MouseSensitivityMax;

    // Value to save/load from settings
    static constexpr const TCHAR* SensitivityConfigSection = TEXT("InputWidget");
    static constexpr const TCHAR* SensitivityConfigKey = TEXT("MouseSensitivity");

    UFUNCTION()
    void OnMouseSensitivityChanged(float Value);

    void UpdateTexts();
    void LoadSensitivity();
    void SaveSensitivity();
};