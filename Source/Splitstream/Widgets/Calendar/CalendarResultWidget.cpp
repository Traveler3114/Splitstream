#include "CalendarResultWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UCalendarResultWidget::SetupResult(const FString& StaffName, UTexture2D* ComputerTexture)
{
    if (StaffNameText)
        StaffNameText->SetText(FText::FromString(StaffName));
    if (ComputerImage && ComputerTexture)
        ComputerImage->SetBrushFromTexture(ComputerTexture);
}