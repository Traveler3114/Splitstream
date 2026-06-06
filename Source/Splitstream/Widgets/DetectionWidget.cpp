#include "DetectionWidget.h"
#include "Components/ProgressBar.h"
#include "Styling/SlateBrush.h"

void UDetectionWidget::SetDetectionProgress(float Progress, bool bIsLocked)
{
    TargetProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
    bIsLockedCached = bIsLocked;

    if (!LerpTimerHandle.IsValid() && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(LerpTimerHandle, this, &UDetectionWidget::TickLerp, 0.016f, true);
    }
}

void UDetectionWidget::TickLerp()
{
    if (!DetectionProgressBar) return;

    const float LerpDt = 0.016f;
    float InterpSpeed = bIsLockedCached ? 20.0f : 15.0f;

    if (bIsLockedCached)
    {
        DisplayedProgress = 1.0f;
    }
    else
    {
        DisplayedProgress = FMath::FInterpTo(DisplayedProgress, TargetProgress, LerpDt, InterpSpeed);
    }

    float Progress = FMath::Clamp(DisplayedProgress, 0.0f, 1.0f);

    FProgressBarStyle BarStyle = DetectionProgressBar->GetWidgetStyle();

    if (Progress >= 1.0f || bIsLockedCached)
    {
        BarStyle.FillImage = FullBrush;
        BarStyle.BackgroundImage = FSlateNoResource();
        DetectionProgressBar->SetWidgetStyle(BarStyle);
        DetectionProgressBar->SetPercent(1.0f);
        DetectionProgressBar->SetFillColorAndOpacity(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f));
    }
    else
    {
        BarStyle.FillImage = NormalBrush;
        BarStyle.BackgroundImage = NormalBackgroundBrush;
        DetectionProgressBar->SetWidgetStyle(BarStyle);
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

    if (!bIsLockedCached && FMath::IsNearlyEqual(DisplayedProgress, TargetProgress, 0.005f))
    {
        DisplayedProgress = TargetProgress;
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(LerpTimerHandle);
        }
    }
}
