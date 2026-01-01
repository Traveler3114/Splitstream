#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnumOptionWidget.generated.h"

class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnumChanged, int32, NewIndex);

UCLASS()
class ECHOESOFTIME_API UEnumOptionWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* LabelText;

    UPROPERTY(meta = (BindWidget))
    UButton* LeftButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ValueText;

    UPROPERTY(meta = (BindWidget))
    UButton* RightButton;

    void Setup(const FText& InLabel, const TArray<FString>& InOptions, int32 InIndex);

    UPROPERTY(BlueprintAssignable)
    FOnEnumChanged OnEnumChanged;

protected:
    virtual void NativeConstruct() override;

private:
    TArray<FString> Options;
    int32 CurrentIndex = 0;

    UFUNCTION()
    void OnLeft();

    UFUNCTION()
    void OnRight();

    void UpdateDisplay();
};