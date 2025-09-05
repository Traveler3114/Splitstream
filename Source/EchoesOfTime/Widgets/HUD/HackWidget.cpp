#include "HackWidget.h"
#include "HackingSystem/HackComponent.h"
#include "Rendering/DrawElements.h"

void UHackWidget::NativeConstruct()
{
    Super::NativeConstruct();
    LastProgress = 0.f;
}

void UHackWidget::InitializeHackWidget(UHackComponent* InHackComp)
{
    HackComp = InHackComp;
    LastProgress = HackComp ? HackComp->GetHackProgress() : 0.f;
}

void UHackWidget::UpdateProgress(float Progress)
{
    LastProgress = Progress;
    Invalidate(EInvalidateWidgetReason::Paint);
}

int32 UHackWidget::NativePaint(const FPaintArgs& Args, const FGeometry& Geometry, const FSlateRect& ClipRect,
    FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    LayerId = Super::NativePaint(Args, Geometry, ClipRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

    const FVector2D Center = Geometry.GetLocalSize() * 0.5f;
    const float Radius = CircleRadius;
    const int32 NumSegments = 64;

    // Draw base circle
    TArray<FVector2D> CirclePoints;
    CirclePoints.Reserve(NumSegments + 1);
    for (int32 i = 0; i < NumSegments; ++i)
    {
        float Angle = 2 * PI * i / NumSegments;
        CirclePoints.Add(Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius);
    }
    if (NumSegments > 0)
    {
        // Store the first point before adding to avoid invalid reference
        const FVector2D FirstCirclePoint = CirclePoints[0];
        CirclePoints.Add(FirstCirclePoint);
    }

    FSlateDrawElement::MakeLines(
        OutDrawElements,
        LayerId++,
        Geometry.ToPaintGeometry(),
        CirclePoints,
        ESlateDrawEffect::None,
        CircleColor,
        true,
        CircleThickness
    );

    // Draw progress arc, just like LockPickWidget
    if (LastProgress > 0.f)
    {
        TArray<FVector2D> ArcPoints;
        int32 FillSegments = FMath::Clamp(FMath::RoundToInt(NumSegments * LastProgress), 1, NumSegments);
        ArcPoints.Reserve(FillSegments + 1);
        for (int32 i = 0; i < FillSegments; ++i)
        {
            float Angle = 2 * PI * i / NumSegments;
            ArcPoints.Add(Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius);
        }
        // Do NOT add the first point again for the arc, to avoid a straight line between start and end

        FSlateDrawElement::MakeLines(
            OutDrawElements,
            LayerId++,
            Geometry.ToPaintGeometry(),
            ArcPoints,
            ESlateDrawEffect::None,
            FillColor,
            true,
            CircleThickness * 1.5f
        );
    }

    return LayerId;
}