#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CalendarResultWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class SPLITSTREAM_API UCalendarResultWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UPROPERTY(meta = (BindWidget))
    UImage* ComputerImage;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* StaffNameText;

    UFUNCTION(BlueprintCallable)
    void SetupResult(const FString& StaffName, UTexture2D* ComputerTexture);
};