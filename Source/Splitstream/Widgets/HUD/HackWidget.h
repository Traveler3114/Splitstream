#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HackWidget.generated.h"

class UHackComponent;

UCLASS()
class ECHOESOFTIME_API UHackWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    // Call after creating the widget to bind to HackingComponent
    UFUNCTION(BlueprintCallable, Category = "Hack")
    void InitializeHackWidget(UHackComponent* InHackComp);

    // Update the widget's display (call every tick with current progress)
    UFUNCTION(BlueprintCallable, Category = "Hack")
    void UpdateProgress(float Progress);

    virtual int32 NativePaint(const FPaintArgs& Args,
        const FGeometry& Geometry,
        const FSlateRect& ClipRect,
        FSlateWindowElementList& OutDrawElements, int32 LayerId,
        const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(EditAnywhere, Category = "Hack|Style")
    FLinearColor CircleColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, Category = "Hack|Style")
    FLinearColor FillColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, Category = "Hack|Style")
    float CircleThickness = 4.f;

    UPROPERTY(EditAnywhere, Category = "Hack|Style")
    float CircleRadius = 120.f;

    UPROPERTY()
    UHackComponent* HackComp = nullptr;

    float LastProgress = 0.f; // 0.0 to 1.0
};