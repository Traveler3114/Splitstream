#include "EnumOptionWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UEnumOptionWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (LeftButton)
    {
        LeftButton->OnClicked.RemoveDynamic(this, &UEnumOptionWidget::OnLeft);
        LeftButton->OnClicked.AddDynamic(this, &UEnumOptionWidget::OnLeft);
    }
    if (RightButton)
    {
        RightButton->OnClicked.RemoveDynamic(this, &UEnumOptionWidget::OnRight);
        RightButton->OnClicked.AddDynamic(this, &UEnumOptionWidget::OnRight);
    }
}

void UEnumOptionWidget::Setup(const FText& InLabel, const TArray<FString>& InOptions, int32 InIndex)
{
    if (LabelText) LabelText->SetText(InLabel);
    Options = InOptions;
    CurrentIndex = InIndex;
    UpdateDisplay();
}

void UEnumOptionWidget::OnLeft()
{
    if (Options.Num() == 0) return;
    CurrentIndex = (CurrentIndex - 1 + Options.Num()) % Options.Num();
    UpdateDisplay();
    OnEnumChanged.Broadcast(CurrentIndex);
}

void UEnumOptionWidget::OnRight()
{
    if (Options.Num() == 0) return;
    CurrentIndex = (CurrentIndex + 1) % Options.Num();
    UpdateDisplay();
    OnEnumChanged.Broadcast(CurrentIndex);
}

void UEnumOptionWidget::UpdateDisplay()
{
    if (ValueText && Options.IsValidIndex(CurrentIndex))
    {
        ValueText->SetText(FText::FromString(Options[CurrentIndex]));
    }
}