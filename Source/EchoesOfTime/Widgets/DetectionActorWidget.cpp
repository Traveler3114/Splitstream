#include "DetectionActorWidget.h"
#include "Components/ProgressBar.h"
#include "Styling/SlateBrush.h"

void UDetectionActorWidget::SetDetectionProgress(float Progress, bool bIsLocked)
{
    if (!DetectionProgressBar) return;
    Progress = FMath::Clamp(Progress, 0.0f, 1.0f);

    // Copy the current style
    FProgressBarStyle BarStyle = DetectionProgressBar->GetWidgetStyle();

    // Switch texture/brush and background based on whether full or not.
    if (Progress >= 1.0f)
    {
        BarStyle.FillImage = FullBrush;
        // Hide background when full
        BarStyle.BackgroundImage = FSlateNoResource();
    }
    else
    {
        BarStyle.FillImage = NormalBrush;
        // Restore background when not full
        BarStyle.BackgroundImage = NormalBackgroundBrush;
    }

    // Set the modified style back
    DetectionProgressBar->SetWidgetStyle(BarStyle);

    if (bIsLocked)
    {
        DetectionProgressBar->SetPercent(1.0f);
        DetectionProgressBar->SetFillColorAndOpacity(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f));
        return;
    }

    DetectionProgressBar->SetPercent(Progress);

    // (You can keep your color interpolation code here as desired)
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