#include "InputWidget.h"
#include "Saving/UserSettingsSaveGame.h"
#include "KeybindWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"

void UInputWidget::NativeConstruct()
{
    Super::NativeConstruct();

    MouseSensitivityMin = 0.1f;
    MouseSensitivityMax = 10.0f;
    MouseSensitivity = 1.0f;
    PendingRebindAction = nullptr;
    PendingRebindWidget = nullptr;

    if (!InputMappingContextRuntime && InputMappingContext)
    {
        InputMappingContextRuntime = DuplicateObject<UInputMappingContext>(InputMappingContext, this);
        if (APlayerController* PC = GetOwningPlayer())
        {
            if (ULocalPlayer* LP = Cast<ULocalPlayer>(PC->Player))
            {
                if (UEnhancedInputLocalPlayerSubsystem* Subsys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
                {
                    Subsys->RemoveMappingContext(InputMappingContext);
                    Subsys->AddMappingContext(InputMappingContextRuntime, 0);
                }
            }
        }
    }

    LoadUserSettings();

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
    if (!KeybindsList || !InputMappingContextRuntime || !KeybindWidgetClass) return;
    KeybindsList->ClearChildren();
    KeybindWidgets.Empty();

    for (const FKeybindDefinition& Def : KeybindsToExpose)
    {
        if (!Def.InputAction) continue;

        // Get key for this input action
        FString KeyStr = TEXT("None");
        for (const FEnhancedActionKeyMapping& Mapping : InputMappingContextRuntime->GetMappings())
        {
            if (Mapping.Action == Def.InputAction)
            {
                KeyStr = Mapping.Key.ToString();
                break;
            }
        }

        UKeybindWidget* RowWidget = CreateWidget<UKeybindWidget>(this, KeybindWidgetClass);
        RowWidget->Setup(
            Def.DisplayName.IsEmpty() ? FText::FromName(Def.InputAction->GetFName()) : Def.DisplayName,
            Def.InputAction,
            FText::FromString(KeyStr)
        );
        RowWidget->OnChangeKeyClicked.AddDynamic(this, &UInputWidget::HandleRowClicked);

        KeybindsList->AddChild(RowWidget);
        KeybindWidgets.Add(RowWidget);
    }
}

void UInputWidget::HandleRowClicked(UKeybindWidget* Source)
{
    // --- Cancel previous pending row if needed
    if (PendingRebindWidget && PendingRebindWidget != Source)
    {
        UpdateKeybindDisplay(PendingRebindWidget->InputAction); // Restore previous row's key string
    }

    PendingRebindAction = Source->InputAction;
    PendingRebindWidget = Source;

    if (Source->KeyInsideButton)
        Source->KeyInsideButton->SetText(FText::FromString(TEXT("Press any key...")));

    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeUIOnly UIOnly;
        PC->SetInputMode(UIOnly);
    }
}

void UInputWidget::UpdateKeybindDisplay(UInputAction* InputAction)
{
    if (!InputMappingContextRuntime || !InputAction) return;
    FString KeyStr = TEXT("None");
    for (const FEnhancedActionKeyMapping& Mapping : InputMappingContextRuntime->GetMappings())
    {
        if (Mapping.Action == InputAction)
        {
            KeyStr = Mapping.Key.ToString();
            break;
        }
    }
    for (UKeybindWidget* Widget : KeybindWidgets)
    {
        if (Widget->InputAction == InputAction && Widget->KeyInsideButton)
        {
            Widget->KeyInsideButton->SetText(FText::FromString(KeyStr));
        }
    }
}

FReply UInputWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (PendingRebindAction && InputMappingContextRuntime)
    {
        FKey NewKey = InKeyEvent.GetKey();
        TArray<FEnhancedActionKeyMapping> OldMappings = InputMappingContextRuntime->GetMappings();
        TArray<FKey> OldKeys;
        for (const FEnhancedActionKeyMapping& Mapping : OldMappings)
        {
            if (Mapping.Action == PendingRebindAction)
            {
                OldKeys.Add(Mapping.Key);
            }
        }
        for (const FKey& OldKey : OldKeys)
        {
            InputMappingContextRuntime->UnmapKey(PendingRebindAction, OldKey);
        }
        if (PendingRebindAction)
        {
            InputMappingContextRuntime->MapKey(PendingRebindAction, NewKey);
        }

        if (APlayerController* PC = GetOwningPlayer())
        {
            if (ULocalPlayer* LP = Cast<ULocalPlayer>(PC->Player))
            {
                if (UEnhancedInputLocalPlayerSubsystem* Subsys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
                {
                    Subsys->RemoveMappingContext(InputMappingContextRuntime);
                    Subsys->AddMappingContext(InputMappingContextRuntime, 0);
                }
            }
        }
        UpdateKeybindDisplay(PendingRebindAction);
        PendingRebindAction = nullptr;
        PendingRebindWidget = nullptr;
        SaveUserSettings();
        return FReply::Handled();
    }
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UInputWidget::OnMouseSensitivityChanged(float Value)
{
    MouseSensitivity = Value;
    UpdateTexts();
    SaveUserSettings();
}

void UInputWidget::UpdateTexts()
{
    if (MouseSensitivityValueText)
        MouseSensitivityValueText->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), MouseSensitivity)));
}

void UInputWidget::ApplySettings()
{
    SaveUserSettings();
}

void UInputWidget::SaveUserSettings()
{
    UUserSettingsSaveGame* SaveGameInstance = Cast<UUserSettingsSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UUserSettingsSaveGame::StaticClass()));
    if (!SaveGameInstance || !InputMappingContextRuntime)
        return;
    SaveGameInstance->SavedKeybinds.Empty();
    for (const FEnhancedActionKeyMapping& Mapping : InputMappingContextRuntime->GetMappings())
    {
        if (Mapping.Action)
        {
            FSavedKeybind Saved;
            Saved.ActionName = Mapping.Action->GetFName();
            Saved.Key = Mapping.Key;
            SaveGameInstance->SavedKeybinds.Add(Saved);
        }
    }
    SaveGameInstance->MouseSensitivity = MouseSensitivity;
    UGameplayStatics::SaveGameToSlot(SaveGameInstance, TEXT("UserSettingsSave"), 0);
}

void UInputWidget::LoadUserSettings()
{
    USaveGame* LoadedGame = UGameplayStatics::LoadGameFromSlot(TEXT("UserSettingsSave"), 0);
    UUserSettingsSaveGame* SaveGameInstance = Cast<UUserSettingsSaveGame>(LoadedGame);
    if (!SaveGameInstance || !InputMappingContextRuntime)
        return;
    for (const FSavedKeybind& Saved : SaveGameInstance->SavedKeybinds)
    {
        UInputAction* Action = nullptr;
        for (const FKeybindDefinition& Def : KeybindsToExpose)
        {
            if (Def.InputAction && Def.InputAction->GetFName() == Saved.ActionName)
            {
                Action = Def.InputAction;
                break;
            }
        }
        if (Action)
        {
            TArray<FEnhancedActionKeyMapping> OldMappings = InputMappingContextRuntime->GetMappings();
            for (const FEnhancedActionKeyMapping& Mapping : OldMappings)
            {
                if (Mapping.Action == Action)
                {
                    InputMappingContextRuntime->UnmapKey(Action, Mapping.Key);
                }
            }
            InputMappingContextRuntime->MapKey(Action, Saved.Key);
        }
    }
    MouseSensitivity = SaveGameInstance->MouseSensitivity;
}