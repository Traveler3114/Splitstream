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