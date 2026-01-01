#include "KeybindWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/SizeBox.h"

void UKeybindWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (KeyChangeButton)
    {
        KeyChangeButton->OnClicked.AddDynamic(this, &UKeybindWidget::HandleChangeKeyClicked);
    }
}

void UKeybindWidget::Setup(const FText& DisplayName, UInputAction* InAction, const FText& KeyText)
{
    InputAction = InAction;
    if (DisplayNameText)
        DisplayNameText->SetText(DisplayName);
    if (KeyInsideButton)
        KeyInsideButton->SetText(KeyText);
}

void UKeybindWidget::HandleChangeKeyClicked()
{
    OnChangeKeyClicked.Broadcast(this);
}