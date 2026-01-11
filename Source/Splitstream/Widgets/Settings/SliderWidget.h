#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SliderWidget.generated.h"

class UTextBlock;
class USlider;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSliderValueChanged, float, Value);

UCLASS()
class SPLITSTREAM_API USliderWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* LabelText;

    UPROPERTY(meta = (BindWidget))
    USlider* Slider;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ValueText;

    void Setup(const FText& InLabel, float Min, float Max, float Value, int32 Precision = 2);

    UPROPERTY(BlueprintAssignable)
    FOnSliderValueChanged OnValueChanged;

    UFUNCTION()
    void HandleSliderChanged(float Value);

protected:
    virtual void NativeConstruct() override;

private:
    int32 ValuePrecision = 2;
    void UpdateValueText(float Value);
};