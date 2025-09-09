#include "DefaultPlayerController.h"
#include "LockPickingSystem/LockPickComponent.h"
#include "Widgets/HUD/CharacterHUD.h"
#include "Widgets/PauseMenuWidget.h"
#include "Widgets/Calendar/CalendarWidget.h"
#include "Engine/Engine.h"

ADefaultPlayerController::ADefaultPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
    CharacterHUD = nullptr;
    PauseMenuWidget = nullptr;
}

void ADefaultPlayerController::BeginPlay()
{
    Super::BeginPlay();
    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;
    if (CharacterHUD)
    {
        if (CharacterHUD->CharacterOverlay == nullptr)
            CharacterHUD->AddCharacterOverlay();
    }
}

void ADefaultPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    BindPauseMenuEsc();
}

void ADefaultPlayerController::BindPauseMenuEsc()
{
    if (InputComponent)
    {
        InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ADefaultPlayerController::TogglePauseMenu);
    }
}

void ADefaultPlayerController::UnbindPauseMenuEsc()
{
    if (InputComponent)
    {
        for (int32 i = InputComponent->KeyBindings.Num() - 1; i >= 0; --i)
        {
            const FInputKeyBinding& KeyBinding = InputComponent->KeyBindings[i];
            if (KeyBinding.Chord.Key == EKeys::Escape && KeyBinding.KeyEvent == IE_Pressed)
            {
                InputComponent->KeyBindings.RemoveAt(i);
            }
        }
    }
}

void ADefaultPlayerController::TogglePauseMenu()
{
    if (!bIsPauseMenuOpen)
    {
        if (!PauseMenuWidget && PauseMenuWidgetClass)
        {
            PauseMenuWidget = CreateWidget<UPauseMenuWidget>(this, PauseMenuWidgetClass);
            PauseMenuWidget->OnPauseMenuResumed.AddDynamic(this, &ADefaultPlayerController::HandlePauseMenuResumed);
        }
        if (PauseMenuWidget && !PauseMenuWidget->IsInViewport())
        {
            PauseMenuWidget->AddToViewport();

            FInputModeGameAndUI InputMode;
            InputMode.SetWidgetToFocus(PauseMenuWidget->TakeWidget());
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            SetInputMode(InputMode);
            bShowMouseCursor = true;
            SetIgnoreMoveInput(true);
            SetIgnoreLookInput(true);
        }
        bIsPauseMenuOpen = true;
    }
    else
    {
        if (PauseMenuWidget && PauseMenuWidget->IsInViewport())
        {
            PauseMenuWidget->RemoveFromParent();

            FInputModeGameOnly InputMode;
            SetInputMode(InputMode);
            bShowMouseCursor = false;
            SetIgnoreMoveInput(false);
            SetIgnoreLookInput(false);
        }
        bIsPauseMenuOpen = false;
    }
}

void ADefaultPlayerController::HandlePauseMenuResumed()
{
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = false;
    SetIgnoreMoveInput(false);
    SetIgnoreLookInput(false);
    bIsPauseMenuOpen = false;
}

void ADefaultPlayerController::ServerLockPickConfirm_Implementation(AActor* DoorActor, float Angle)
{
    if (!DoorActor) return;
    ULockPickComponent* LockComp = DoorActor->FindComponentByClass<ULockPickComponent>();
    if (LockComp)
    {
        if (LockComp->TrySetCurrentPin(Angle))
        {
            if (LockComp->AdvancePin())
            {
                LockComp->EndLockPicking();
            }
        }
    }
}


void ADefaultPlayerController::ClientShowCalendarWidget_Implementation(int32 Year, int32 Month, int32 Day, const TArray<FString>& StaffNames)
{
    if (CalendarWidgetClass)
    {
        UCalendarWidget* Widget = CreateWidget<UCalendarWidget>(this, CalendarWidgetClass);
        if (Widget)
        {
            Widget->TargetYear = Year;
            Widget->TargetMonth = Month;
            Widget->TargetDay = Day;
            Widget->StaffNames = StaffNames; // New: pass array
            Widget->AddToViewport();
        }
    }
}
