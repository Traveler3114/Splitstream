#include "CalendarWidget.h"
#include "CalendarButtonWidget.h"
#include "CalendarResultWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void UCalendarWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true);
    ShowYearButtons();

    // Set input focus for key events
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(PC, this, EMouseLockMode::DoNotLock);
        PC->bShowMouseCursor = true;
    }

    CalendarStage = ECalendarStage::Years;
}

FReply UCalendarWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::Escape)
    {
        GoBack();
        return FReply::Handled();
    }
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UCalendarWidget::ShowYearButtons()
{
    CalendarStage = ECalendarStage::Years;

    if (!CalendarPanel || !CalendarButtonWidgetClass) return;
    CalendarPanel->ClearChildren();
    SelectedYear = 0;
    SelectedMonth = 0;
    SelectedDay = 0;

    int32 StartYear = 1990;
    int32 EndYear = 2025;
    int32 NumCols = 5;
    int32 TotalYears = EndYear - StartYear + 1;
    int32 NumRows = FMath::CeilToInt((float)TotalYears / NumCols);

    for (int32 Year = StartYear; Year <= EndYear; ++Year)
    {
        UCalendarButtonWidget* YearBtn = CreateWidget<UCalendarButtonWidget>(GetWorld(), CalendarButtonWidgetClass);
        if (YearBtn)
        {
            YearBtn->Init(Year, 0, 0);
            YearBtn->OnCalendarButtonClicked.AddDynamic(this, &UCalendarWidget::OnCalendarButtonClicked);

            int32 Index = Year - StartYear;
            int32 Row = Index % NumRows;
            int32 Col = Index / NumRows;

            CalendarPanel->AddChildToUniformGrid(YearBtn, Col, Row);
        }
    }
}

void UCalendarWidget::ShowMonthButtons(int32 Year)
{
    CalendarStage = ECalendarStage::Months;

    if (!CalendarPanel || !CalendarButtonWidgetClass) return;
    CalendarPanel->ClearChildren();
    SelectedMonth = 0;
    SelectedDay = 0;

    int32 NumCols = 4;
    int32 TotalMonths = 12;
    int32 NumRows = FMath::CeilToInt((float)TotalMonths / NumCols);

    for (int32 Month = 1; Month <= 12; ++Month)
    {
        UCalendarButtonWidget* MonthBtn = CreateWidget<UCalendarButtonWidget>(GetWorld(), CalendarButtonWidgetClass);
        if (MonthBtn)
        {
            MonthBtn->Init(Year, Month, 0);
            MonthBtn->OnCalendarButtonClicked.AddDynamic(this, &UCalendarWidget::OnCalendarButtonClicked);

            int32 Index = Month - 1;
            int32 Row = Index % NumRows;
            int32 Col = Index / NumRows;

            CalendarPanel->AddChildToUniformGrid(MonthBtn, Col, Row);
        }
    }
}

void UCalendarWidget::ShowDayButtons(int32 Year, int32 Month)
{
    CalendarStage = ECalendarStage::Days;

    if (!CalendarPanel || !CalendarButtonWidgetClass) return;
    CalendarPanel->ClearChildren();
    SelectedDay = 0;

    int32 DaysInMonth = 31;
    if (Month == 2)
    {
        DaysInMonth = 28;
        if ((Year % 4 == 0 && Year % 100 != 0) || (Year % 400 == 0)) DaysInMonth = 29;
    }
    else if (Month == 4 || Month == 6 || Month == 9 || Month == 11)
    {
        DaysInMonth = 30;
    }

    int32 NumCols = 7;
    int32 NumRows = FMath::CeilToInt((float)DaysInMonth / NumCols);

    for (int32 Day = 1; Day <= DaysInMonth; ++Day)
    {
        UCalendarButtonWidget* DayBtn = CreateWidget<UCalendarButtonWidget>(GetWorld(), CalendarButtonWidgetClass);
        if (DayBtn)
        {
            DayBtn->Init(Year, Month, Day);
            DayBtn->OnCalendarButtonClicked.AddDynamic(this, &UCalendarWidget::OnCalendarButtonClicked);

            int32 Index = Day - 1;
            int32 Row = Index % NumRows;
            int32 Col = Index / NumRows;

            CalendarPanel->AddChildToUniformGrid(DayBtn, Col, Row);
        }
    }
}

void UCalendarWidget::ShowResult()
{
    CalendarStage = ECalendarStage::Results;
    CalendarPanel->ClearChildren();

    if (SelectedYear == TargetYear && SelectedMonth == TargetMonth && SelectedDay == TargetDay)
    {
        if (CalendarResultWidgetClass)
        {
            UCalendarResultWidget* ResultWidget = CreateWidget<UCalendarResultWidget>(GetWorld(), CalendarResultWidgetClass);
            if (ResultWidget)
            {
                ResultWidget->SetupResult(TargetStaffName, ComputerTexture);
                CalendarPanel->AddChildToUniformGrid(ResultWidget, 0, 0);
            }
        }
    }
    else
    {
        UTextBlock* NotFound = NewObject<UTextBlock>(this);
        NotFound->SetText(FText::FromString(TEXT("No record found for this date.")));
        CalendarPanel->AddChildToUniformGrid(NotFound, 0, 0);
    }
    SetKeyboardFocus();
}

void UCalendarWidget::GoBack()
{
    switch (CalendarStage)
    {
    case ECalendarStage::Results:
        ShowDayButtons(SelectedYear, SelectedMonth);
        break;
    case ECalendarStage::Days:
        ShowMonthButtons(SelectedYear);
        break;
    case ECalendarStage::Months:
        ShowYearButtons();
        break;
    case ECalendarStage::Years:
        CloseCalendar();
        break;
    default:
        CloseCalendar();
        break;
    }
}

void UCalendarWidget::CloseCalendar()
{
    RemoveFromParent();
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
}

void UCalendarWidget::OnCalendarButtonClicked(int32 Year, int32 Month, int32 Day)
{
    if (Year > 0 && Month == 0 && Day == 0)
    {
        SelectedYear = Year;
        ShowMonthButtons(SelectedYear);
    }
    else if (Year > 0 && Month > 0 && Day == 0)
    {
        SelectedMonth = Month;
        ShowDayButtons(SelectedYear, SelectedMonth);
    }
    else if (Year > 0 && Month > 0 && Day > 0)
    {
        SelectedDay = Day;
        ShowResult();
    }
}