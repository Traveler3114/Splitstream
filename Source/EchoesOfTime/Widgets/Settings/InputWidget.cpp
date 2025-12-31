#include "InputWidget.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Engine/Engine.h"

void UInputWidget::NativeConstruct()
{
    Super::NativeConstruct();

    MouseSensitivityMin = 0.1f;
    MouseSensitivityMax = 10.0f;
    MouseSensitivity = 1.0f;
    PendingRebindAction = nullptr;

    LoadSensitivity();

    if (MouseSensitivitySlider)
    {
        MouseSensitivitySlider->SetMinValue(MouseSensitivityMin);
        MouseSensitivitySlider->SetMaxValue(MouseSensitivityMax);
        MouseSensitivitySlider->SetValue(MouseSensitivity);
        MouseSensitivitySlider->OnValueChanged.RemoveDynamic(this, &UInputWidget::OnMouseSensitivityChanged);
        MouseSensitivitySlider->OnValueChanged.AddDynamic(this, &UInputWidget::OnMouseSensitivityChanged);
    }

    BuildKeybindList();
    UpdateTexts();
}

void UInputWidget::BuildKeybindList()
{
    if (!KeybindsList || !InputMappingContext) return;
    KeybindsList->ClearChildren();
    KeyRows.Empty();

    for (const FKeybindDefinition& Def : KeybindsToExpose)
    {
        if (!Def.InputAction) continue;

        FText RowDisplayName = Def.DisplayName.IsEmpty()
            ? FText::FromName(Def.InputAction->GetFName())
            : Def.DisplayName;
        UInputAction* InputAction = Def.InputAction;

        UHorizontalBox* Row = NewObject<UHorizontalBox>(this);

        // Label
        UTextBlock* Label = NewObject<UTextBlock>(this);
        Label->SetText(RowDisplayName);
        Label->SetJustification(ETextJustify::Left);

        UHorizontalBoxSlot* LabelSlot = Row->AddChildToHorizontalBox(Label);
        LabelSlot->SetPadding(FMargin(2, 2, 16, 2));
        LabelSlot->SetHorizontalAlignment(HAlign_Left);
        LabelSlot->SetSize(ESlateSizeRule::Automatic);

        // Keybind button
        UButton* KeyButton = NewObject<UButton>(this);

        UTextBlock* KeyButtonText = NewObject<UTextBlock>(KeyButton);
        KeyButtonText->SetText(FText::FromString(TEXT("Key")));
        KeyButtonText->SetJustification(ETextJustify::Center);
        KeyButton->AddChild(KeyButtonText);

        UHorizontalBoxSlot* BtnSlot = Row->AddChildToHorizontalBox(KeyButton);
        BtnSlot->SetPadding(FMargin(2, 2, 2, 2));
        BtnSlot->SetHorizontalAlignment(HAlign_Fill);
        BtnSlot->SetSize(ESlateSizeRule::Fill);

        KeyButton->OnClicked.AddDynamic(this, &UInputWidget::OnChangeKeyClicked);

        KeybindsList->AddChild(Row);

        FKeybindRowWidgets RowWidgets;
        RowWidgets.DisplayNameLabel = Label;
        RowWidgets.ChangeKeyButton = KeyButton;
        RowWidgets.KeyInsideButton = KeyButtonText;
        RowWidgets.InputAction = InputAction;
        KeyRows.Add(RowWidgets);

        UpdateKeybindDisplay(InputAction);
    }
}

void UInputWidget::UpdateKeybindDisplay(UInputAction* InputAction)
{
    if (!InputMappingContext || !InputAction) return;
    for (FKeybindRowWidgets& Row : KeyRows)
    {
        if (Row.InputAction == InputAction)
        {
            FString KeyStr = TEXT("None");
            for (const FEnhancedActionKeyMapping& Mapping : InputMappingContext->GetMappings())
            {
                if (Mapping.Action == InputAction)
                {
                    KeyStr = Mapping.Key.ToString();
                    break;
                }
            }
            if (Row.KeyInsideButton)
                Row.KeyInsideButton->SetText(FText::FromString(KeyStr));
            return;
        }
    }
}

void UInputWidget::OnChangeKeyClicked()
{
    for (FKeybindRowWidgets& Row : KeyRows)
    {
        if (Row.ChangeKeyButton && Row.ChangeKeyButton->HasKeyboardFocus())
        {
            PendingRebindAction = Row.InputAction;
            if (Row.KeyInsideButton)
                Row.KeyInsideButton->SetText(FText::FromString(TEXT("Press any key...")));
            break;
        }
    }
    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeUIOnly UIOnly;
        PC->SetInputMode(UIOnly);
    }
}

FReply UInputWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (PendingRebindAction && InputMappingContext)
    {
        FKey NewKey = InKeyEvent.GetKey();

        // Workaround for UE < 5.2: Remove and readd
        TArray<FEnhancedActionKeyMapping> OldMappings = InputMappingContext->GetMappings();
        TArray<const UInputAction*> ToRemove;
        for (const FEnhancedActionKeyMapping& Mapping : OldMappings)
        {
            if (Mapping.Action == PendingRebindAction)
            {
                ToRemove.Add(Mapping.Action);
            }
        }
        for (const UInputAction* MappingAction : ToRemove)
        {
            InputMappingContext->UnmapKey(MappingAction, EKeys::AnyKey);
        }
        // Add new mapping
        const UInputAction* ActionToMap = PendingRebindAction;
        if (ActionToMap)
        {
            InputMappingContext->MapKey(ActionToMap, NewKey);
        }

        // Apply live to subsystem
        if (APlayerController* PC = GetOwningPlayer())
        {
            if (ULocalPlayer* LP = Cast<ULocalPlayer>(PC->Player))
            {
                if (UEnhancedInputLocalPlayerSubsystem* Subsys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
                {
                    Subsys->RemoveMappingContext(InputMappingContext);
                    Subsys->AddMappingContext(InputMappingContext, 0);
                }
            }
        }
        UpdateKeybindDisplay(PendingRebindAction);
        PendingRebindAction = nullptr;
        return FReply::Handled();
    }
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UInputWidget::OnMouseSensitivityChanged(float Value)
{
    MouseSensitivity = Value;
    UpdateTexts();
}

void UInputWidget::UpdateTexts()
{
    if (MouseSensitivityValueText)
        MouseSensitivityValueText->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), MouseSensitivity)));
}

void UInputWidget::ApplySettings()
{
    SaveSensitivity();
}

void UInputWidget::SaveSensitivity()
{
    GConfig->SetFloat(
        SensitivityConfigSection,
        SensitivityConfigKey,
        MouseSensitivity,
        GGameUserSettingsIni
    );
}

void UInputWidget::LoadSensitivity()
{
    float LoadedSensitivity = 1.0f;
    if (GConfig->GetFloat(SensitivityConfigSection, SensitivityConfigKey, LoadedSensitivity, GGameUserSettingsIni))
    {
        MouseSensitivity = FMath::Clamp(LoadedSensitivity, MouseSensitivityMin, MouseSensitivityMax);
    }
}