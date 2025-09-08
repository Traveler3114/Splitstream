// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/UniformGridPanel.h"
#include "ArchiveCalendarWidget.generated.h"

// Forward declarations
class AArchiveComputer;

USTRUCT(BlueprintType)
struct FCalendarEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calendar")
    FString Date;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calendar")
    FString Content;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calendar")
    bool bIsHighlighted = false;

    FCalendarEntry()
    {
        Date = TEXT("");
        Content = TEXT("");
        bIsHighlighted = false;
    }
};

UCLASS()
class ECHOESOFTIME_API UArchiveCalendarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // Initialize widget with archive computer reference
    UFUNCTION(BlueprintCallable, Category = "Archive")
    void InitializeWidget(AArchiveComputer* InArchiveComputer);

    // Update the calendar display
    UFUNCTION(BlueprintCallable, Category = "Archive")
    void UpdateCalendar();

    // Navigate calendar months
    UFUNCTION(BlueprintCallable, Category = "Archive")
    void NextMonth();

    UFUNCTION(BlueprintCallable, Category = "Archive")
    void PreviousMonth();

    // Select a date
    UFUNCTION(BlueprintCallable, Category = "Archive")
    void SelectDate(const FString& Date);

    // Close the archive interface
    UFUNCTION(BlueprintCallable, Category = "Archive")
    void CloseArchive();

protected:
    // UI Components - can be bound in Blueprint
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TitleText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* MonthYearText;

    UPROPERTY(meta = (BindWidget))
    UUniformGridPanel* CalendarGrid;

    UPROPERTY(meta = (BindWidget))
    UScrollBox* EntryScrollBox;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* SelectedDateText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* EntryContentText;

    UPROPERTY(meta = (BindWidget))
    UButton* PrevMonthButton;

    UPROPERTY(meta = (BindWidget))
    UButton* NextMonthButton;

    UPROPERTY(meta = (BindWidget))
    UButton* CloseButton;

    // Reference to the archive computer
    UPROPERTY()
    AArchiveComputer* ArchiveComputer = nullptr;

    // Calendar state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Calendar")
    int32 CurrentMonth = 3; // March (0-based)

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Calendar")
    int32 CurrentYear = 2157;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Calendar")
    FString SelectedDate;

    // Archive entries
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Calendar")
    TArray<FCalendarEntry> CalendarEntries;

    // Button event handlers
    UFUNCTION()
    void OnPrevMonthClicked();

    UFUNCTION()
    void OnNextMonthClicked();

    UFUNCTION()
    void OnCloseClicked();

    // Create calendar day buttons
    void CreateCalendarDays();

    // Update entry display
    void UpdateEntryDisplay();

    // Convert archive data to calendar entries
    void PopulateCalendarEntries();

    // Get month name
    FString GetMonthName(int32 Month) const;

    // Get number of days in month
    int32 GetDaysInMonth(int32 Month, int32 Year) const;
};