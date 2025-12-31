#include "InputWidget.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UInputWidget::NativeConstruct()
{
    Super::NativeConstruct();

    MouseSensitivityMin = 0.1f;
    MouseSensitivityMax = 10.0f;
    MouseSensitivity = 1.0f;

    // Try to load previous value (optional)
    LoadSensitivity();

    if (MouseSensitivitySlider)
    {
        MouseSensitivitySlider->SetMinValue(MouseSensitivityMin);
        MouseSensitivitySlider->SetMaxValue(MouseSensitivityMax);
        MouseSensitivitySlider->SetValue(MouseSensitivity);

        MouseSensitivitySlider->OnValueChanged.RemoveDynamic(this, &UInputWidget::OnMouseSensitivityChanged);
        MouseSensitivitySlider->OnValueChanged.AddDynamic(this, &UInputWidget::OnMouseSensitivityChanged);
    }

    UpdateTexts();
}

void UInputWidget::OnMouseSensitivityChanged(float Value)
{
    MouseSensitivity = Value;
    UpdateTexts();
}

void UInputWidget::UpdateTexts()
{
    if (MouseSensitivityValueText)
    {
        // Show with 2 decimal places
        MouseSensitivityValueText->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), MouseSensitivity)));
    }
}

void UInputWidget::ApplySettings()
{
    // Save sensitivity (you would use this value in your actual input processing)
    SaveSensitivity();
}

// Optional: store in config file so the value persists (for demo; you can use SaveGame instead if you prefer)
void UInputWidget::SaveSensitivity()
{
    GConfig->SetFloat(
        SensitivityConfigSection,
        SensitivityConfigKey,
        MouseSensitivity,
        GGameUserSettingsIni
    );
}

void UInputWidget::LoadSensitivity()
{
    float LoadedSensitivity = 1.0f;
    if (GConfig->GetFloat(SensitivityConfigSection, SensitivityConfigKey, LoadedSensitivity, GGameUserSettingsIni))
    {
        MouseSensitivity = FMath::Clamp(LoadedSensitivity, MouseSensitivityMin, MouseSensitivityMax);
    }
}