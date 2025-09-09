#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CalendarWidget.generated.h"

class UUniformGridPanel;
class UCalendarButtonWidget;
class UCalendarResultWidget;

UENUM()
enum class ECalendarStage : uint8
{
    Years,
    Months,
    Days,
    Results
};

UCLASS()
class ECHOESOFTIME_API UCalendarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    UPROPERTY(meta = (BindWidget))
    UUniformGridPanel* CalendarPanel;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Calendar")
    TSubclassOf<UCalendarButtonWidget> CalendarButtonWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Calendar")
    TSubclassOf<UCalendarResultWidget> CalendarResultWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calendar")
    int32 TargetYear = 2020;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calendar")
    int32 TargetMonth = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calendar")
    int32 TargetDay = 1;

    // Array of staff/civilian names to show multiple computers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calendar")
    TArray<FString> CivilianNames;

    // Array of portraits for each civilian
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calendar")
    TArray<UTexture2D*> CivilianPortraits;

private:
    int32 SelectedYear = 0;
    int32 SelectedMonth = 0;
    int32 SelectedDay = 0;

    ECalendarStage CalendarStage = ECalendarStage::Years;

    void ShowYearButtons();
    void ShowMonthButtons(int32 Year);
    void ShowDayButtons(int32 Year, int32 Month);
    void ShowResult();

    void GoBack();
    void CloseCalendar();

    UFUNCTION()
    void OnCalendarButtonClicked(int32 Year, int32 Month, int32 Day);
};