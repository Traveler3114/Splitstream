#include "DetectionWidget.h"
#include "Components/ProgressBar.h"

void UDetectionWidget::SetDetectionProgress(float Progress, bool bIsLocked)
{
    if (!DetectionProgressBar) return;
    Progress = FMath::Clamp(Progress, 0.0f, 1.0f);

    if (bIsLocked)
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

void UDetectionWidget::SetDetectionBarAngle(float AngleDegrees)
{
    if (DetectionProgressBar)
    {
        DetectionProgressBar->SetRenderTransformAngle(AngleDegrees);
    }
}