#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CalendarButtonWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCalendarButtonClicked, int32, Year, int32, Month, int32, Day);

UCLASS()
class SPLITSTREAM_API UCalendarButtonWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UPROPERTY(meta = (BindWidget))
    UButton* Calendar_btn;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* CalendarText;

    void Init(int32 InYear, int32 InMonth, int32 InDay);

    UPROPERTY(BlueprintAssignable)
    FOnCalendarButtonClicked OnCalendarButtonClicked;

protected:
    virtual void NativeConstruct() override;

private:
    int32 Year;
    int32 Month;
    int32 Day;

    UFUNCTION()
    void HandleButtonClicked();
};