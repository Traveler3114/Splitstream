// Fill out your copyright notice in the Description page of Project Settings.

#include "ArchiveCalendarWidget.h"
#include "../../Actors/ArchiveComputer.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/UniformGridPanel.h"
#include "Components/Border.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/Engine.h"

void UArchiveCalendarWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind button events
    if (PrevMonthButton)
    {
        PrevMonthButton->OnClicked.AddDynamic(this, &UArchiveCalendarWidget::OnPrevMonthClicked);
    }

    if (NextMonthButton)
    {
        NextMonthButton->OnClicked.AddDynamic(this, &UArchiveCalendarWidget::OnNextMonthClicked);
    }

    if (CloseButton)
    {
        CloseButton->OnClicked.AddDynamic(this, &UArchiveCalendarWidget::OnCloseClicked);
    }

    // Set initial title
    if (TitleText)
    {
        TitleText->SetText(FText::FromString(TEXT("Temporal Archive System")));
    }
}

void UArchiveCalendarWidget::NativeDestruct()
{
    // Clean up button bindings
    if (PrevMonthButton)
    {
        PrevMonthButton->OnClicked.RemoveAll(this);
    }

    if (NextMonthButton)
    {
        NextMonthButton->OnClicked.RemoveAll(this);
    }

    if (CloseButton)
    {
        CloseButton->OnClicked.RemoveAll(this);
    }

    Super::NativeDestruct();
}

void UArchiveCalendarWidget::InitializeWidget(AArchiveComputer* InArchiveComputer)
{
    ArchiveComputer = InArchiveComputer;
    
    if (ArchiveComputer)
    {
        // Populate calendar entries from archive computer
        PopulateCalendarEntries();
        
        // Update the calendar display
        UpdateCalendar();
        
        UE_LOG(LogTemp, Log, TEXT("Archive calendar widget initialized"));
    }
}

void UArchiveCalendarWidget::UpdateCalendar()
{
    // Update month/year display
    if (MonthYearText)
    {
        FString MonthYear = FString::Printf(TEXT("%s %d"), *GetMonthName(CurrentMonth), CurrentYear);
        MonthYearText->SetText(FText::FromString(MonthYear));
    }

    // Create calendar day buttons
    CreateCalendarDays();

    // Update entry display if a date is selected
    if (!SelectedDate.IsEmpty())
    {
        UpdateEntryDisplay();
    }
}

void UArchiveCalendarWidget::NextMonth()
{
    CurrentMonth++;
    if (CurrentMonth > 11)
    {
        CurrentMonth = 0;
        CurrentYear++;
    }
    UpdateCalendar();
}

void UArchiveCalendarWidget::PreviousMonth()
{
    CurrentMonth--;
    if (CurrentMonth < 0)
    {
        CurrentMonth = 11;
        CurrentYear--;
    }
    UpdateCalendar();
}

void UArchiveCalendarWidget::SelectDate(const FString& Date)
{
    SelectedDate = Date;
    
    if (SelectedDateText)
    {
        SelectedDateText->SetText(FText::FromString(FString::Printf(TEXT("Selected: %s"), *Date)));
    }
    
    UpdateEntryDisplay();
}

void UArchiveCalendarWidget::CloseArchive()
{
    if (ArchiveComputer)
    {
        ArchiveComputer->CloseArchiveInterface();
    }
}

void UArchiveCalendarWidget::OnPrevMonthClicked()
{
    PreviousMonth();
}

void UArchiveCalendarWidget::OnNextMonthClicked()
{
    NextMonth();
}

void UArchiveCalendarWidget::OnCloseClicked()
{
    CloseArchive();
}

void UArchiveCalendarWidget::CreateCalendarDays()
{
    if (!CalendarGrid)
    {
        return;
    }

    // Clear existing calendar buttons
    CalendarGrid->ClearChildren();

    // Get number of days in current month
    int32 DaysInMonth = GetDaysInMonth(CurrentMonth, CurrentYear);

    // Create day buttons in a grid (7 columns for days of week)
    for (int32 Day = 1; Day <= DaysInMonth; ++Day)
    {
        // Create date string in format YYYY.MM.DD
        FString DateString = FString::Printf(TEXT("%04d.%02d.%02d"), CurrentYear, CurrentMonth + 1, Day);
        
        // Create button for this day
        UButton* DayButton = NewObject<UButton>(this);
        if (DayButton)
        {
            // Create text block for the day number
            UTextBlock* DayText = NewObject<UTextBlock>(this);
            if (DayText)
            {
                DayText->SetText(FText::AsNumber(Day));
                
                // Check if this date has an entry
                bool bHasEntry = false;
                for (const FCalendarEntry& Entry : CalendarEntries)
                {
                    if (Entry.Date == DateString)
                    {
                        bHasEntry = true;
                        // Highlight text for days with entries
                        DayText->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
                        break;
                    }
                }
                
                if (!bHasEntry)
                {
                    DayText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
                }
                
                DayButton->AddChild(DayText);
            }
            
            // Add button to grid
            int32 Row = (Day - 1) / 7;
            int32 Column = (Day - 1) % 7;
            CalendarGrid->AddChildToUniformGrid(DayButton, Row, Column);
            
            // Bind click event (would need custom implementation for dynamic binding)
            // For now, we'll handle selection through Blueprint or other means
        }
    }
}

void UArchiveCalendarWidget::UpdateEntryDisplay()
{
    if (!SelectedDate.IsEmpty() && EntryContentText)
    {
        // Find entry for selected date
        FString EntryContent;
        bool bFoundEntry = false;
        
        for (const FCalendarEntry& Entry : CalendarEntries)
        {
            if (Entry.Date == SelectedDate)
            {
                EntryContent = Entry.Content;
                bFoundEntry = true;
                break;
            }
        }
        
        if (bFoundEntry)
        {
            EntryContentText->SetText(FText::FromString(EntryContent));
        }
        else
        {
            EntryContentText->SetText(FText::FromString(TEXT("No entry for this date.")));
        }
    }
}

void UArchiveCalendarWidget::PopulateCalendarEntries()
{
    CalendarEntries.Empty();
    
    if (!ArchiveComputer)
    {
        return;
    }
    
    // Get archive entries from computer
    TArray<FString> ArchiveDates = ArchiveComputer->GetArchiveEntries();
    
    for (const FString& Date : ArchiveDates)
    {
        FString Content = ArchiveComputer->GetArchiveEntryByDate(Date);
        
        if (!Content.IsEmpty())
        {
            FCalendarEntry NewEntry;
            NewEntry.Date = Date;
            NewEntry.Content = Content;
            NewEntry.bIsHighlighted = true; // Highlight entries with content
            
            CalendarEntries.Add(NewEntry);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Populated %d calendar entries"), CalendarEntries.Num());
}

FString UArchiveCalendarWidget::GetMonthName(int32 Month) const
{
    static TArray<FString> MonthNames = {
        TEXT("January"), TEXT("February"), TEXT("March"), TEXT("April"),
        TEXT("May"), TEXT("June"), TEXT("July"), TEXT("August"),
        TEXT("September"), TEXT("October"), TEXT("November"), TEXT("December")
    };
    
    if (Month >= 0 && Month < MonthNames.Num())
    {
        return MonthNames[Month];
    }
    
    return TEXT("Unknown");
}

int32 UArchiveCalendarWidget::GetDaysInMonth(int32 Month, int32 Year) const
{
    static TArray<int32> DaysPerMonth = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    
    if (Month >= 0 && Month < DaysPerMonth.Num())
    {
        int32 Days = DaysPerMonth[Month];
        
        // Handle leap year for February
        if (Month == 1) // February
        {
            if ((Year % 4 == 0 && Year % 100 != 0) || (Year % 400 == 0))
            {
                Days = 29;
            }
        }
        
        return Days;
    }
    
    return 30; // Default
}