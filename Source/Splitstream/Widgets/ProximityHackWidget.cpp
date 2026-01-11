// ProximityHackWidget.cpp
#include "ProximityHackWidget.h"
#include "Components/ProgressBar.h"

void UProximityHackWidget::SetHackProgress(float Progress)
{
    if (ProgressBar)
    {
        ProgressBar->SetPercent(FMath::Clamp(Progress, 0.0f, 1.0f));
    }
}