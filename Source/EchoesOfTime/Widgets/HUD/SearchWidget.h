#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SearchWidget.generated.h"

class USearchComponent;

UCLASS()
class ECHOESOFTIME_API USearchWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    // Call after creating the widget to bind to SearchingComponent
    UFUNCTION(BlueprintCallable, Category = "Search")
    void InitializeSearchWidget(USearchComponent* InSearchComp);

    // Update the widget's display (call every tick with current progress)
    UFUNCTION(BlueprintCallable, Category = "Search")
    void UpdateProgress(float Progress);

    virtual int32 NativePaint(const FPaintArgs& Args,
        const FGeometry& Geometry,
        const FSlateRect& ClipRect,
        FSlateWindowElementList& OutDrawElements, int32 LayerId,
        const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(EditAnywhere, Category = "Search|Style")
    FLinearColor CircleColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, Category = "Search|Style")
    FLinearColor FillColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, Category = "Search|Style")
    float CircleThickness = 4.f;

    UPROPERTY(EditAnywhere, Category = "Search|Style")
    float CircleRadius = 120.f;

    UPROPERTY()
    USearchComponent* SearchComp = nullptr;

    float LastProgress = 0.f; // 0.0 to 1.0
};