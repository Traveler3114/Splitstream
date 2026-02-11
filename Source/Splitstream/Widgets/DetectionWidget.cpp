#include "DetectionWidget.h"
#include "Components/ProgressBar.h"
#include "Styling/SlateBrush.h"

void UDetectionWidget::SetDetectionProgress(float Progress, bool bIsLocked)
{
    TargetProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
    bIsLockedCached = bIsLocked;
    // Do NOT call SetPercent here!
}


void UDetectionWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Choose an interp speed. 8-12 is a good starting value
    float InterpSpeed = bIsLockedCached ? 20.0f : 8.0f; // Snap fast if locked

    // If locked, snap to 1.0f immediately. Else, lerp.
    if (bIsLockedCached)
    {
        DisplayedProgress = 1.0f;
    }
    else
    {
        DisplayedProgress = FMath::FInterpTo(DisplayedProgress, TargetProgress, InDeltaTime, InterpSpeed);
    }

    // Now update the bar
    if (DetectionProgressBar)
    {
        // Apply same color/brush logic as before...
        float Progress = FMath::Clamp(DisplayedProgress, 0.0f, 1.0f);

        FProgressBarStyle BarStyle = DetectionProgressBar->GetWidgetStyle();

        if (Progress >= 1.0f)
        {
            BarStyle.FillImage = FullBrush;
            BarStyle.BackgroundImage = FSlateNoResource();
        }
        else
        {
            BarStyle.FillImage = NormalBrush;
            BarStyle.BackgroundImage = NormalBackgroundBrush;
        }
        DetectionProgressBar->SetWidgetStyle(BarStyle);

        if (bIsLockedCached)
        {
            DetectionProgressBar->SetPercent(1.0f);
            DetectionProgressBar->SetFillColorAndOpacity(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f));
            return;
        }

        DetectionProgressBar->SetPercent(Progress);

        FLinearColor BarColor;
        if (Progress < 0.5f)
        {
            float T = Progress / 0.5f;
            BarColor = FLinearColor::LerpUsingHSV(FLinearColor(0.0f, 1.0f, 0.0f, 1.0f), FLinearColor(1.0f, 1.0f, 0.0f, 1.0f), T);
        }
        else if (Progress < 0.75f)
        {
            float T = (Progress - 0.5f) / 0.25f;
            BarColor = FLinearColor::LerpUsingHSV(FLinearColor(1.0f, 1.0f, 0.0f, 1.0f), FLinearColor(1.0f, 0.5f, 0.0f, 1.0f), T);
        }
        else
        {
            float T = (Progress - 0.75f) / 0.25f;
            BarColor = FLinearColor::LerpUsingHSV(FLinearColor(1.0f, 0.5f, 0.0f, 1.0f), FLinearColor(1.0f, 0.0f, 0.0f, 1.0f), T);
        }
        DetectionProgressBar->SetFillColorAndOpacity(BarColor);
    }
}