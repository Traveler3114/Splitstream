#include "TabButton.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UTabButton::NativeConstruct()
{
    Super::NativeConstruct();

    if (ButtonRoot)
    {
        ButtonRoot->OnClicked.AddDynamic(this, &UTabButton::HandleClicked);
    }
}

void UTabButton::HandleClicked()
{
    OnTabButtonClicked.Broadcast(TabIndex);
}

void UTabButton::SetTabLabel(const FText& InLabel)
{
    // If LabelText exists in the widget, set it automatically
    if (LabelText)
    {
        LabelText->SetText(InLabel);
    }

    // Also fire the Blueprint event so custom layouts can react
    OnLabelSet(InLabel);
}

void UTabButton::SetActive(bool bActive)
{
    bIsActive = bActive;

    // Fire the Blueprint event — handle all visuals there
    OnActiveStateChanged(bIsActive);
}
