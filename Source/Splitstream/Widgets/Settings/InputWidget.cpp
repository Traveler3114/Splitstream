#include "InputWidget.h"
#include "Saving/UserSettingsSaveGame.h"
#include "KeybindWidget.h"
#include "SliderWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "DefaultGameInstance.h"

void UInputWidget::NativeConstruct()
{
    Super::NativeConstruct();

    MouseSensitivityMin = 0.1f;
    MouseSensitivityMax = 10.0f;
    MouseSensitivity    = 1.0f;
    PendingRebindAction = nullptr;
    PendingRebindWidget = nullptr;

    LoadUserSettingsFromGameInstance();
    SetupWidgets();
    BuildKeybindList();
    UpdateTexts();
}

void UInputWidget::SetupWidgets()
{
    if (MouseSensitivityWidget)
    {
        MouseSensitivityWidget->Setup(
            FText::FromString(TEXT("Mouse Sensitivity")),
            MouseSensitivityMin, MouseSensitivityMax, MouseSensitivity, 2
        );
        MouseSensitivityWidget->OnValueChanged.RemoveDynamic(this, &UInputWidget::OnMouseSensitivityChanged);
        MouseSensitivityWidget->OnValueChanged.AddDynamic(this, &UInputWidget::OnMouseSensitivityChanged);
    }
}

void UInputWidget::OnMouseSensitivityChanged(float Value)
{
    MouseSensitivity = Value;
    UpdateTexts();
    SaveUserSettingsToGameInstance();
}

void UInputWidget::BuildKeybindList()
{
    UDefaultGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<UDefaultGameInstance>() : nullptr;
    if (!KeybindsList || !GI || !GI->GetCurrentInputMappingContext() || !KeybindWidgetClass) return;

    KeybindsList->ClearChildren();
    KeybindWidgets.Empty();

    for (const FKeybindDefinition& Def : KeybindsToExpose)
    {
        if (!Def.InputAction) continue;

        FString KeyStr = TEXT("None");
        for (const FEnhancedActionKeyMapping& Mapping : GI->GetCurrentInputMappingContext()->GetMappings())
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
    PendingRebindAction = Source->InputAction;
    PendingRebindWidget = Source;

    if (Source->KeyInsideButton)
        Source->KeyInsideButton->SetText(FText::FromString(TEXT("Press any key...")));

    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeUIOnly UIOnly;
        PC->SetInputMode(UIOnly);
        SetUserFocus(PC);
        SetKeyboardFocus();
    }
}

void UInputWidget::UpdateKeybindDisplay(UInputAction* InputAction)
{
    UDefaultGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<UDefaultGameInstance>() : nullptr;
    if (!GI || !GI->GetCurrentInputMappingContext() || !InputAction) return;

    FString KeyStr = TEXT("None");
    for (const FEnhancedActionKeyMapping& Mapping : GI->GetCurrentInputMappingContext()->GetMappings())
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
    UDefaultGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<UDefaultGameInstance>() : nullptr;
    if (!GI || !GI->GetCurrentInputMappingContext())
        return Super::NativeOnKeyDown(InGeometry, InKeyEvent);

    if (PendingRebindAction)
    {
        FKey NewKey = InKeyEvent.GetKey();
        UInputMappingContext* MappingContext = GI->GetCurrentInputMappingContext();

        for (const FEnhancedActionKeyMapping& Mapping : TArray<FEnhancedActionKeyMapping>(MappingContext->GetMappings()))
        {
            if (Mapping.Action == PendingRebindAction)
                MappingContext->UnmapKey(PendingRebindAction, Mapping.Key);
        }

        if (GI->DefaultMappingContext && GI->DefaultMappingContext != MappingContext)
        {
            for (const FEnhancedActionKeyMapping& Mapping : TArray<FEnhancedActionKeyMapping>(GI->DefaultMappingContext->GetMappings()))
            {
                if (Mapping.Action == PendingRebindAction)
                    GI->DefaultMappingContext->UnmapKey(PendingRebindAction, Mapping.Key);
            }
        }

        MappingContext->MapKey(PendingRebindAction, NewKey);

        if (APlayerController* PC = GetOwningPlayer())
        {
            if (ULocalPlayer* LP = Cast<ULocalPlayer>(PC->Player))
            {
                if (UEnhancedInputLocalPlayerSubsystem* Subsys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
                {
                    Subsys->ClearAllMappings();
                    Subsys->AddMappingContext(MappingContext, 0);
                }
            }
        }

        UpdateKeybindDisplay(PendingRebindAction);
        PendingRebindAction = nullptr;
        PendingRebindWidget = nullptr;
        SaveUserSettingsToGameInstance();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UInputWidget::UpdateTexts() { /* slider widget handles its own value text */ }

// ─────────────────────────────────────────────────────────────────────────────
//  ISettingsTabInterface implementation
// ─────────────────────────────────────────────────────────────────────────────
void UInputWidget::ApplySettings_Implementation()
{
    SaveUserSettingsToGameInstance();
}

void UInputWidget::SaveUserSettingsToGameInstance()
{
    UDefaultGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<UDefaultGameInstance>() : nullptr;
    if (!GI || !GI->GetCurrentInputMappingContext()) return;

    TArray<FSavedKeybind> Keybinds;
    for (const FEnhancedActionKeyMapping& Mapping : GI->GetCurrentInputMappingContext()->GetMappings())
    {
        if (Mapping.Action)
        {
            FSavedKeybind Saved;
            Saved.ActionName = Mapping.Action->GetFName();
            Saved.Key        = Mapping.Key;
            Keybinds.Add(Saved);
        }
    }

    GI->SaveUserSettings(Keybinds, MouseSensitivity);
    GI->LoadUserSettings();
}

void UInputWidget::LoadUserSettingsFromGameInstance()
{
    UDefaultGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<UDefaultGameInstance>() : nullptr;
    MouseSensitivity = GI ? GI->GetMouseSensitivity() : 1.0f;

    if (MouseSensitivityWidget)
    {
        MouseSensitivityWidget->Setup(
            FText::FromString(TEXT("Mouse Sensitivity")),
            MouseSensitivityMin, MouseSensitivityMax, MouseSensitivity, 2
        );
    }
}
