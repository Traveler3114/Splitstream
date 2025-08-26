#include "LockPickWidget.h"
#include "LockPickingSystem/LockPickComponent.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void ULockPickWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // Pin images created dynamically in InitializeLockPickWidget
}

#include "Slate/SlateBrushAsset.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Rendering/DrawElements.h"

// ... existing code ...

int32 ULockPickWidget::NativePaint(const FPaintArgs& Args, const FGeometry& Geometry, const FSlateRect& ClipRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    LayerId = Super::NativePaint(Args, Geometry, ClipRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

    // Draw the lockpick dial
    const FVector2D Center = Geometry.GetLocalSize() * 0.5f;
    const float Radius = 180.f;
    const int32 NumSegments = 64;
    const FLinearColor CircleColor = FLinearColor::White;
    const float Thickness = 2.f;

    // Preallocate and fill the circle points
    TArray<FVector2D> CirclePoints;
    CirclePoints.Reserve(NumSegments + 1);
    for (int32 i = 0; i < NumSegments; ++i)
    {
        float Angle = 2 * PI * i / NumSegments;
        CirclePoints.Add(Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius);
    }
    if (NumSegments > 0)
    {
        const FVector2D FirstPoint = CirclePoints[0];
        CirclePoints.Add(FirstPoint);
    }

    FSlateDrawElement::MakeLines(
        OutDrawElements,
        LayerId++,
        Geometry.ToPaintGeometry(),
        CirclePoints,
        ESlateDrawEffect::None,
        CircleColor,
        true,
        Thickness
    );

    // Draw the angle indicator
    float AngleRad = FMath::DegreesToRadians(LastInputAngle);
    const float StartOffset = 80.f; // How far from the center to start
    FVector2D Direction(FMath::Cos(AngleRad), FMath::Sin(AngleRad));
    FVector2D IndicatorStart = Center + Direction * StartOffset;
    FVector2D IndicatorEnd = Center + Direction * Radius;
    FSlateDrawElement::MakeLines(
        OutDrawElements,
        LayerId++,
        Geometry.ToPaintGeometry(),
        { IndicatorStart, IndicatorEnd },
        ESlateDrawEffect::None,
        FLinearColor::Red,
        true,
        4.f
    );

    return LayerId;
}

void ULockPickWidget::InitializeLockPickWidget(ULockPickComponent* InLockComp)
{
    LockComp = InLockComp;
    if (!PinContainer || !LockComp)
        return;

    PinContainer->ClearChildren();
    PinImages.Empty();

    int32 PinCount = LockComp->GetPinCount();
    for (int32 i = 0; i < PinCount; ++i)
    {
        UImage* PinImg = NewObject<UImage>(this, PinImageClass ? *PinImageClass : UImage::StaticClass());
        if (PinImg && PinImageTexture)
        {
            FSlateBrush Brush;
            Brush.SetResourceObject(PinImageTexture);
            PinImg->SetBrush(Brush);
            PinImg->SetColorAndOpacity(InactivePinColor);
            PinImg->SetDesiredSizeOverride(FVector2D(48.f, 48.f)); // Larger pin size
        }

        UHorizontalBoxSlot* SSlot = Cast<UHorizontalBoxSlot>(PinContainer->AddChildToHorizontalBox(PinImg));
        if (SSlot)
        {
            SSlot->SetPadding(FMargin(8.f, 0.f)); // Add horizontal spacing
            SSlot->SetSize(ESlateSizeRule::Automatic);
        }

        PinImages.Add(PinImg);
    }
    UpdatePins(0.f);
}

void ULockPickWidget::UpdatePins(float CurrentInputAngle)
{
    LastInputAngle = CurrentInputAngle;
    if (!LockComp)
        return;

    int32 ActivePin = LockComp->GetActivePinIndex();
    int32 NumPins = LockComp->GetPinCount();

    for (int32 i = 0; i < PinImages.Num(); ++i)
    {
        UImage* PinImg = PinImages[i];
        if (!PinImg)
            continue;

        if (LockComp->PinSetStates.IsValidIndex(i) && LockComp->PinSetStates[i])
        {
            PinImg->SetColorAndOpacity(SetPinColor);
            PinImg->SetOpacity(1.f);
        }
        else if (i == ActivePin)
        {
            float Proximity = LockComp->GetPinAngleProximity(i, CurrentInputAngle);
            PinImg->SetColorAndOpacity(ActivePinColor);
            PinImg->SetOpacity(0.2f + 0.8f * Proximity); // More pronounced fade
        }
        else
        {
            PinImg->SetColorAndOpacity(InactivePinColor);
            PinImg->SetOpacity(0.5f);
        }
    }
}