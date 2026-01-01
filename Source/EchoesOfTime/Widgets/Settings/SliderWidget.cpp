#include "SliderWidget.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"

void USliderWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (Slider)
    {
        Slider->OnValueChanged.RemoveDynamic(this, &USliderWidget::HandleSliderChanged);
        Slider->OnValueChanged.AddDynamic(this, &USliderWidget::HandleSliderChanged);
    }
}

void USliderWidget::Setup(const FText& InLabel, float Min, float Max, float Value, int32 Precision)
{
    if (LabelText) LabelText->SetText(InLabel);
    if (Slider)
    {
        Slider->SetMinValue(Min);
        Slider->SetMaxValue(Max);
        Slider->SetValue(Value);
    }
    ValuePrecision = Precision;
    UpdateValueText(Value);
}

void USliderWidget::HandleSliderChanged(float InValue)
{
    UpdateValueText(InValue);
    OnValueChanged.Broadcast(InValue);
}

void USliderWidget::UpdateValueText(float Value)
{
    if (ValueText)
    {
        FString NumString = FString::Printf(
            TEXT("%.*f"), ValuePrecision, Value
        );
        ValueText->SetText(FText::FromString(NumString));
    }
}