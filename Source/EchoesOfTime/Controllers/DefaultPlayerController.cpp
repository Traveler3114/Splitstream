// DefaultPlayerController.cpp
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
#include "AbilitySystem/EOTGameplayTags.h"
#include "GameStates/DefaultGameState.h"
#include "GameplayEffectTypes.h"
#include "TimerManager.h"

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
    BindAttributeDelegates();
    BindGameplayTagDelegates();
    GetWorldTimerManager().SetTimer(PingUpdateTimerHandle, this, &ADefaultPlayerController::UpdatePingOnOverlay, 1.0f, true);

    // Bind to GameState alarm events - only for local controllers (to update local UI)
    if (IsLocalController())
    {
        if (ADefaultGameState* GS = GetWorld() ? GetWorld()->GetGameState<ADefaultGameState>() : nullptr)
        {
            GS->OnAlarmStarted.AddDynamic(this, &ADefaultPlayerController::HandleAlarmStarted);
            GS->OnAlarmCanceled.AddDynamic(this, &ADefaultPlayerController::HandleAlarmCanceled);

            // If an alarm is already active by the time we join, initialize from the replicated values:
            if (GS->bAlarmActive && GS->AlarmEndTime > 0.f)
            {
                HandleAlarmStarted(GS->AlarmEndTime);
            }
            else
            {
                // Ensure UI is clear if no alarm is active
                HandleAlarmCanceled();
            }
        }
    }
}

void ADefaultPlayerController::HandleAlarmStarted(float InAlarmEndTime)
{
    // store end time and start a small timer to update UI frequently (0.1s)
    AlarmEndTime = InAlarmEndTime;

    // ensure existing timer cleared
    GetWorldTimerManager().ClearTimer(AlarmUpdateTimerHandle);
    GetWorldTimerManager().SetTimer(AlarmUpdateTimerHandle, this, &ADefaultPlayerController::UpdateAlarmUI, 0.1f, true);

    // Immediately update UI once
    UpdateAlarmUI();
}

void ADefaultPlayerController::HandleAlarmCanceled()
{
    // Stop local timer and clear UI immediately
    AlarmEndTime = 0.f;
    GetWorldTimerManager().ClearTimer(AlarmUpdateTimerHandle);

    if (CharacterHUD && CharacterHUD->CharacterOverlay)
    {
        CharacterHUD->CharacterOverlay->SetStatusTextWithColor(TEXT(""), FLinearColor::White);
    }
}

void ADefaultPlayerController::UpdateAlarmUI()
{
    if (!CharacterHUD || !CharacterHUD->CharacterOverlay) return;

    float Now = GetWorld()->GetTimeSeconds();
    float Remaining = FMath::Max(0.f, AlarmEndTime - Now);

    // Format text - you can tweak presentation here
    int32 SecondsLeft = FMath::CeilToInt(Remaining);
    FString StatusText = FString::Printf(TEXT("ALARM - Restart in %d s"), SecondsLeft);

    // Set status text on overlay in RED for full alarm
    CharacterHUD->CharacterOverlay->SetStatusTextWithColor(StatusText, FLinearColor::Red);

    // Stop when done
    if (Remaining <= 0.f)
    {
        GetWorldTimerManager().ClearTimer(AlarmUpdateTimerHandle);
        // Clear status text when done
        CharacterHUD->CharacterOverlay->SetStatusTextWithColor(TEXT(""), FLinearColor::White);
        AlarmEndTime = 0.f;
    }
}

// --- Pre-alarm client RPCs & handlers ---

// Implementation must match header parameter name (InPreAlarmEndTime)
void ADefaultPlayerController::ClientStartPreAlarm_Implementation(float InPreAlarmEndTime)
{
    HandlePreAlarmStarted(InPreAlarmEndTime);
}

void ADefaultPlayerController::ClientCancelPreAlarm_Implementation()
{
    HandlePreAlarmCanceled();
}

void ADefaultPlayerController::HandlePreAlarmStarted(float InPreAlarmEndTime)
{
    PreAlarmEndTime = InPreAlarmEndTime;

    // make sure any previous timer is cleared
    GetWorldTimerManager().ClearTimer(PreAlarmUpdateTimerHandle);
    GetWorldTimerManager().SetTimer(PreAlarmUpdateTimerHandle, this, &ADefaultPlayerController::UpdatePreAlarmUI, 0.1f, true);

    UpdatePreAlarmUI();
}

void ADefaultPlayerController::HandlePreAlarmCanceled()
{
    PreAlarmEndTime = 0.f;
    GetWorldTimerManager().ClearTimer(PreAlarmUpdateTimerHandle);

    if (CharacterHUD && CharacterHUD->CharacterOverlay)
    {
        CharacterHUD->CharacterOverlay->SetStatusTextWithColor(TEXT(""), FLinearColor::White);
    }
}

void ADefaultPlayerController::UpdatePreAlarmUI()
{
    if (!CharacterHUD || !CharacterHUD->CharacterOverlay) return;

    float Now = GetWorld()->GetTimeSeconds();
    float Remaining = FMath::Max(0.f, PreAlarmEndTime - Now);

    int32 SecondsLeft = FMath::CeilToInt(Remaining);
    FString StatusText = FString::Printf(TEXT("Guard spotted you! Alarm in %d s"), SecondsLeft);

    // Set pre-alarm text to YELLOW as a warning
    CharacterHUD->CharacterOverlay->SetStatusTextWithColor(StatusText, FLinearColor::Yellow);

    if (Remaining <= 0.f)
    {
        GetWorldTimerManager().ClearTimer(PreAlarmUpdateTimerHandle);
        // Clear pre-alarm UI; GameState->StartAlarm will arrive shortly and trigger full alarm UI
        CharacterHUD->CharacterOverlay->SetStatusTextWithColor(TEXT(""), FLinearColor::White);
        PreAlarmEndTime = 0.f;
    }
}

void ADefaultPlayerController::UpdatePingOnOverlay()
{
    if (CharacterHUD && CharacterHUD->CharacterOverlay)
    {
        APlayerState* PS = GetPlayerState<APlayerState>();
        if (PS)
        {
            float PingMs = PS->ExactPing;
            CharacterHUD->CharacterOverlay->SetPingText(PingMs);
        }
    }
}

void ADefaultPlayerController::ClientShowLoadingScreen_Implementation()
{
    if (LoadingWidgetClass)
    {
        // Assign outside the if condition to avoid C4706
        LoadingWidgetInstance = CreateWidget<UUserWidget>(this, LoadingWidgetClass, TEXT("LoadingWidget"));
        if (LoadingWidgetInstance)
        {
            LoadingWidgetInstance->AddToViewport();
        }
    }
}

void ADefaultPlayerController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    BindAttributeDelegates();
    BindGameplayTagDelegates();
}

void ADefaultPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();
    if (PS)
    {
        if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
        {
            ASC->RegisterGameplayTagEvent(TAG_Character_Status_Illegal, EGameplayTagEventType::NewOrRemoved)
                .Remove(IllegalTagDelegateHandle);
        }
    }
    GetWorldTimerManager().ClearTimer(PingUpdateTimerHandle);
    GetWorldTimerManager().ClearTimer(AlarmUpdateTimerHandle);
    GetWorldTimerManager().ClearTimer(PreAlarmUpdateTimerHandle);
    Super::EndPlay(EndPlayReason);
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
    if (CalendarWidgetInstance && CalendarWidgetInstance->IsInViewport())
    {
        // Widget is already open; don't create another!
        return;
    }

    if (CalendarWidgetClass)
    {
        CalendarWidgetInstance = CreateWidget<UCalendarWidget>(this, CalendarWidgetClass);
        if (CalendarWidgetInstance)
        {
            CalendarWidgetInstance->CivilianDateRecords = CivilianDateRecords;
            CalendarWidgetInstance->AddToViewport();
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

void ADefaultPlayerController::BindAttributeDelegates()
{
    ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();
    if (!PS) return;
    UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
    UPlayerAttributeSet* AttrSet = PS->GetAttributeSet();

    if (!ASC || !AttrSet || !CharacterHUD || !CharacterHUD->CharacterOverlay) return;

    ASC->GetGameplayAttributeValueChangeDelegate(AttrSet->GetHealthAttribute())
        .AddUObject(this, &ADefaultPlayerController::OnHealthChanged);

    CharacterHUD->CharacterOverlay->SetHealthText(AttrSet->GetHealth());
}

void ADefaultPlayerController::OnHealthChanged(const FOnAttributeChangeData& Data)
{
    if (CharacterHUD && CharacterHUD->CharacterOverlay)
    {
        CharacterHUD->CharacterOverlay->SetHealthText(Data.NewValue);
    }
}

void ADefaultPlayerController::BindGameplayTagDelegates()
{
    ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();
    if (!PS || !CharacterHUD || !CharacterHUD->CharacterOverlay) return;

    UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
    if (!ASC) return;

    FGameplayTag IllegalTag = TAG_Character_Status_Illegal;
    IllegalTagDelegateHandle = ASC->RegisterGameplayTagEvent(IllegalTag, EGameplayTagEventType::NewOrRemoved)
        .AddUObject(this, &ADefaultPlayerController::OnIllegalTagChanged);
}

void ADefaultPlayerController::OnIllegalTagChanged(const FGameplayTag Tag, int32 NewCount)
{
    if (CharacterHUD && CharacterHUD->CharacterOverlay)
    {
        if (NewCount > 0)
        {
            // Keep illegal status RED
            CharacterHUD->CharacterOverlay->SetStatusTextWithColor(TEXT("Illegal"), FLinearColor::Red);
        }
        else
        {
            CharacterHUD->CharacterOverlay->SetStatusTextWithColor(TEXT(""), FLinearColor::White);
        }
    }
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