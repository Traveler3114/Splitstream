#include "CalendarButtonWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UCalendarButtonWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (Calendar_btn)
    {
        Calendar_btn->OnClicked.AddDynamic(this, &UCalendarButtonWidget::HandleButtonClicked);
    }
}

void UCalendarButtonWidget::Init(int32 InYear, int32 InMonth, int32 InDay)
{
    Year = InYear;
    Month = InMonth;
    Day = InDay;

    static const TCHAR* ShortMonthNames[12] = {
        TEXT("Jan"), TEXT("Feb"), TEXT("Mar"), TEXT("Apr"),
        TEXT("May"), TEXT("Jun"), TEXT("Jul"), TEXT("Aug"),
        TEXT("Sep"), TEXT("Oct"), TEXT("Nov"), TEXT("Dec")
    };

    FString Display;
    if (Year > 0 && Month == 0 && Day == 0)
        Display = FString::FromInt(Year); // Show year
    else if (Year > 0 && Month > 0 && Day == 0 && Month >= 1 && Month <= 12)
        Display = ShortMonthNames[Month - 1]; // Show month name
    else if (Year > 0 && Month > 0 && Day > 0)
        Display = FString::FromInt(Day); // Show day
    else
        Display = TEXT("");

    if (CalendarText)
    {
        CalendarText->SetText(FText::FromString(Display));
    }
}

void UCalendarButtonWidget::HandleButtonClicked()
{
    OnCalendarButtonClicked.Broadcast(Year, Month, Day);
}