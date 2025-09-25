#include "DefaultPlayerController.h"
#include "ActorComponents/LockPickComponent.h"
#include "Widgets/HUD/CharacterHUD.h"
#include "Widgets/PauseMenuWidget.h"
#include "Widgets/Calendar/CalendarWidget.h"
#include "Engine/Engine.h"
#include "DefaultPlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/PlayerAttributeSet.h"
#include "Widgets/HUD/CharacterOverlay.h"
#include "GameplayEffectTypes.h"

ADefaultPlayerController::ADefaultPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
    CharacterHUD = nullptr;
    PauseMenuWidget = nullptr;
}

void ADefaultPlayerController::ServerTryLockPick_Implementation(AActor* TargetDoor, float Angle)
{
    if (TargetDoor)
    {
        if (ULockPickComponent* LockComp = TargetDoor->FindComponentByClass<ULockPickComponent>())
        {
            LockComp->ServerTrySetPin(Angle);
        }
    }
}

void ADefaultPlayerController::BindAttributeDelegates()
{
    ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();
    if (!PS) return;
    UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
    UPlayerAttributeSet* AttrSet = Cast<UPlayerAttributeSet>(PS->GetAttributeSet());
    if (!ASC || !AttrSet || !CharacterHUD || !CharacterHUD->CharacterOverlay) return;

    ASC->GetGameplayAttributeValueChangeDelegate(AttrSet->GetHealthAttribute())
        .AddUObject(this, &ADefaultPlayerController::OnHealthChanged);

    // Set initial value
    CharacterHUD->CharacterOverlay->SetHealthText(AttrSet->GetHealth());
}

void ADefaultPlayerController::OnHealthChanged(const FOnAttributeChangeData& Data)
{
    if (CharacterHUD && CharacterHUD->CharacterOverlay)
    {
        CharacterHUD->CharacterOverlay->SetHealthText(Data.NewValue);
    }
}

void ADefaultPlayerController::OnRep_PlayerState()
{
    BindAttributeDelegates();
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
    BindAttributeDelegates();
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

void ADefaultPlayerController::ClientShowCalendarWidget_Implementation(const TArray<FCalendarCivilianRecord>& CivilianDateRecords)
{
    if (CalendarWidgetClass)
    {
        UCalendarWidget* Widget = CreateWidget<UCalendarWidget>(this, CalendarWidgetClass);
        if (Widget)
        {
            Widget->CivilianDateRecords = CivilianDateRecords;
            Widget->AddToViewport();
        }
    }
}

void ADefaultPlayerController::ClientUpdateDetectionWidgetForGuard_Implementation(AActor* Guard, float Progress, bool bIsLocked, float AngleDegrees)
{
    if (CharacterHUD && CharacterHUD->CharacterOverlay)
    {
        CharacterHUD->CharacterOverlay->UpdateDetectionWidgetForGuard(Guard, Progress, bIsLocked, AngleDegrees);
    }
}