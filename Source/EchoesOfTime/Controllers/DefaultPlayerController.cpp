#include "DefaultPlayerController.h"
#include "ActorComponents/LockPickComponent.h"
#include "Widgets/HUD/CharacterHUD.h"
#include "Widgets/PauseMenuWidget.h"
#include "Widgets/Calendar/CalendarWidget.h"
#include "Engine/Engine.h"
#include "DefaultPlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/PlayerAttributeSet.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Widgets/HUD/CharacterOverlay.h"
#include "AbilitySystem/EOTGameplayTags.h"
#include "ActorComponents/DetectionComponent.h"
#include "GameStates/DefaultGameState.h"
#include "GameplayEffectTypes.h"
#include "Actors/RepairableBase.h"
#include "Interfaces/IServerActionInterface.h"
#include "Actors/Terminal.h"
#include "GameFramework/Character.h"
#include "GameModes/DefaultGameMode.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"

ADefaultPlayerController::ADefaultPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
    CharacterHUD = nullptr;
    PauseMenuWidget = nullptr;
}

void ADefaultPlayerController::SetupOverlay()
{
    if (!IsLocalController()) return;

    CharacterHUD = Cast<ACharacterHUD>(GetHUD());
    if (CharacterHUD)
    {
        CharacterHUD->AddCharacterOverlay();

        // Only proceed if overlay instance actually exists
        if (CharacterHUD->CharacterOverlay)
        {
            // Ping
            if (APlayerState* PS = GetPlayerState<APlayerState>())
                CharacterHUD->CharacterOverlay->SetPingText(PS->ExactPing);

            // Health
            if (ADefaultPlayerState* MyPS = Cast<ADefaultPlayerState>(GetPlayerState<APlayerState>()))
                if (UPlayerAttributeSet* AttrSet = MyPS->GetAttributeSet())
                    CharacterHUD->CharacterOverlay->SetHealthText(AttrSet->GetHealth());

            // Money/Objective
            if (ADefaultGameState* GS = GetWorld()->GetGameState<ADefaultGameState>())
                CharacterHUD->CharacterOverlay->SetObjectiveText(GS->CurrentMoneyCollected, GS->TargetMoneyAmount);
        }
    }

    // Always bind events/delegates after overlay creation
    BindAttributeDelegates();
    BindGameplayTagDelegates();
    GetWorldTimerManager().SetTimer(PingUpdateTimerHandle, this, &ADefaultPlayerController::UpdatePingOnOverlay, 1.0f, true);

    // Alarm/pre-alarm event handlers and immediate state update
    if (ADefaultGameState* GS = GetWorld() ? GetWorld()->GetGameState<ADefaultGameState>() : nullptr)
    {
        GS->OnAlarmStarted.RemoveDynamic(this, &ADefaultPlayerController::HandleAlarmStarted);
        GS->OnAlarmStarted.AddDynamic(this, &ADefaultPlayerController::HandleAlarmStarted);
        GS->OnAlarmCanceled.RemoveDynamic(this, &ADefaultPlayerController::HandleAlarmCanceled);
        GS->OnAlarmCanceled.AddDynamic(this, &ADefaultPlayerController::HandleAlarmCanceled);
        GS->OnPreAlarmStarted.RemoveDynamic(this, &ADefaultPlayerController::HandlePreAlarmStarted);
        GS->OnPreAlarmStarted.AddDynamic(this, &ADefaultPlayerController::HandlePreAlarmStarted);
        GS->OnPreAlarmCanceled.RemoveDynamic(this, &ADefaultPlayerController::HandlePreAlarmCanceled);
        GS->OnPreAlarmCanceled.AddDynamic(this, &ADefaultPlayerController::HandlePreAlarmCanceled);
        GS->OnMoneyCollectedChanged.RemoveDynamic(this, &ADefaultPlayerController::OnMoneyCollectedChanged);
        GS->OnMoneyCollectedChanged.AddDynamic(this, &ADefaultPlayerController::OnMoneyCollectedChanged);
        GS->OnGuardRepairETAStarted.RemoveDynamic(this, &ADefaultPlayerController::HandleRepairETAStarted);
        GS->OnGuardRepairETAStarted.AddDynamic(this, &ADefaultPlayerController::HandleRepairETAStarted);
        OnMoneyCollectedChanged(GS->CurrentMoneyCollected, GS->TargetMoneyAmount);


        // --- The critical part: immediately update alarm/pre-alarm UI
        if (GS->bAlarmActive && GS->AlarmEndTime > 0.f)
        {
            HandleAlarmStarted(GS->AlarmEndTime);
        }
        else if (GS->bPreAlarmActive && GS->PreAlarmEndTime > 0.f)
        {
            HandlePreAlarmStarted(GS->PreAlarmEndTime, GS->PreAlarmInstigator);
        }
        else
        {
            HandleAlarmCanceled();
            HandlePreAlarmCanceled();
        }
    }
}

void ADefaultPlayerController::RequestLeaveToMainMenu()
{
    if (HasAuthority() && IsLocalController())
    {
        // Host: tell GameMode
        if (ADefaultGameMode* GM = GetWorld()->GetAuthGameMode<ADefaultGameMode>())
        {
            GM->HostLeaveLobby();
        }
    }
    else
    {
        if (ADefaultGameState* GS = GetWorld()->GetGameState<ADefaultGameState>())
        {
            ClientShowLoadingScreen();
            ClientTravel(GS->MainMenuMapPath, TRAVEL_Absolute);
        }
    }
}

void ADefaultPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    SetupOverlay();
}

void ADefaultPlayerController::BeginPlay()
{
    Super::BeginPlay();
    SetupOverlay();
}

void ADefaultPlayerController::HandleRepairETAStarted(ARepairableBase* Repairable, float Duration)
{
    if (!Repairable || !CharacterHUD || !CharacterHUD->CharacterOverlay) return;
    AActor* CompletionTarget = Repairable->CompletionTarget;
    float EndTime = GetWorld()->GetTimeSeconds() + Duration;
    ActiveRepairCountdowns.Add(CompletionTarget, EndTime);

    // Start timer if not already running
    if (!GetWorldTimerManager().IsTimerActive(RepairCountdownsUpdateTimer))
    {
        GetWorldTimerManager().SetTimer(
            RepairCountdownsUpdateTimer,
            this,
            &ADefaultPlayerController::UpdateRepairCountdownUI,
            0.1f,
            true
        );
    }

    CharacterHUD->CharacterOverlay->UpdateRepairCountdowns(ActiveRepairCountdowns);
}

void ADefaultPlayerController::UpdateRepairCountdownUI()
{
    float Now = GetWorld()->GetTimeSeconds();
    TArray<AActor*> ToRemove;

    for (auto& Pair : ActiveRepairCountdowns)
    {
        if (Now >= Pair.Value)
        {
            ToRemove.Add(Pair.Key);
        }
    }
    for (AActor* Target : ToRemove)
    {
        ActiveRepairCountdowns.Remove(Target);
    }

    // Send updated times to the overlay
    if (CharacterHUD && CharacterHUD->CharacterOverlay)
    {
        CharacterHUD->CharacterOverlay->UpdateRepairCountdowns(ActiveRepairCountdowns);
    }

    // Stop timer if no countdowns remain
    if (ActiveRepairCountdowns.Num() == 0)
    {
        GetWorldTimerManager().ClearTimer(RepairCountdownsUpdateTimer);
    }
}

void ADefaultPlayerController::OnMoneyCollectedChanged(int32 Current, int32 Target)
{
    if (CharacterHUD && CharacterHUD->CharacterOverlay)
    {
        CharacterHUD->CharacterOverlay->SetObjectiveText(Current, Target);
    }
}


void ADefaultPlayerController::HandleAlarmStarted(float InAlarmEndTime)
{
    // If a pre-alarm UI is active, clear it IMMEDIATELY
    HandlePreAlarmCanceled();

    AlarmEndTime = InAlarmEndTime;
    GetWorldTimerManager().ClearTimer(AlarmUpdateTimerHandle);
    GetWorldTimerManager().SetTimer(AlarmUpdateTimerHandle, this, &ADefaultPlayerController::UpdateAlarmUI, 0.1f, true);
    UpdateAlarmUI();
}

void ADefaultPlayerController::HandleAlarmCanceled()
{
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

    int32 SecondsLeft = FMath::FloorToInt(Remaining);
    FString StatusText = FString::Printf(TEXT("ALARM - Restart in %d s"), SecondsLeft);

    CharacterHUD->CharacterOverlay->SetStatusTextWithColor(StatusText, FLinearColor::Red);

    if (Remaining <= 0.f)
    {
        GetWorldTimerManager().ClearTimer(AlarmUpdateTimerHandle);
        CharacterHUD->CharacterOverlay->SetStatusTextWithColor(TEXT(""), FLinearColor::White);
        AlarmEndTime = 0.f;
    }
}

void ADefaultPlayerController::HandlePreAlarmStarted(float InPreAlarmEndTime, AActor* PreAlarmInstigator)
{
    // Only show pre-alarm if full alarm is not active
    if (AlarmEndTime > 0.f)
        return;

    PreAlarmEndTime = InPreAlarmEndTime;
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
        if (AlarmEndTime <= 0.f)
            CharacterHUD->CharacterOverlay->SetStatusTextWithColor(TEXT(""), FLinearColor::White);
    }
}

void ADefaultPlayerController::UpdatePreAlarmUI()
{
    if (!CharacterHUD || !CharacterHUD->CharacterOverlay) return;

    float Now = GetWorld()->GetTimeSeconds();
    float Remaining = FMath::Max(0.f, PreAlarmEndTime - Now);

    int32 SecondsLeft = FMath::FloorToInt(Remaining);
    FString StatusText = FString::Printf(TEXT("Guard spotted you! Alarm in %d s"), SecondsLeft);

    CharacterHUD->CharacterOverlay->SetStatusTextWithColor(StatusText, FLinearColor::Yellow);

    if (Remaining <= 0.f)
    {
        GetWorldTimerManager().ClearTimer(PreAlarmUpdateTimerHandle);
        if (AlarmEndTime <= 0.f)
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
    SetupOverlay();
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
    if (InputComponent)
    {
        InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ADefaultPlayerController::TogglePauseMenu);
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

void ADefaultPlayerController::ClientShowCalendarWidget_Implementation(const TArray<FCalendarDateRecord>& CalendarDateRecords)
{
    if (CalendarWidgetInstance && CalendarWidgetInstance->IsInViewport())
    {
        return;
    }

    if (CalendarWidgetClass)
    {
        CalendarWidgetInstance = CreateWidget<UCalendarWidget>(this, CalendarWidgetClass);
        if (CalendarWidgetInstance)
        {
            CalendarWidgetInstance->CalendarDateRecords = CalendarDateRecords;
            CalendarWidgetInstance->AddToViewport();
        }
    }
}


void ADefaultPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}


void ADefaultPlayerController::ClientUpdateDetectionWidget_Implementation(AActor* Detector, float Progress, bool bIsLocked)
{
    if (!Detector || !CharacterHUD || !CharacterHUD->CharacterOverlay)
        return;

    FVector DetectorLocation = Detector->GetActorLocation();

    // For Character? Use Capsule Height for top
    const ACharacter* Char = Cast<ACharacter>(Detector);
    if (Char)
    {
        const UCapsuleComponent* Capsule = Char->GetCapsuleComponent();
        if (Capsule) {
            DetectorLocation += FVector(0.f, 0.f, Capsule->GetScaledCapsuleHalfHeight());
        }
    }
    else
    {
        const UPrimitiveComponent* Prim = Detector->FindComponentByClass<UPrimitiveComponent>();
        if (Prim) {
            FVector Origin, BoxExtent;
            Detector->GetActorBounds(true, Origin, BoxExtent);
            DetectorLocation = Origin + FVector(0.f, 0.f, BoxExtent.Z);
        }
    }
    DetectorLocation.Z += 20.f;

    // --- Camera facing math ---
    FVector CameraLoc;
    FRotator CameraRot;
    GetPlayerViewPoint(CameraLoc, CameraRot);

    FVector ToTarget = DetectorLocation - CameraLoc;
    ToTarget.Normalize();

    const FVector CameraForward = CameraRot.Vector();
    float Dot = FVector::DotProduct(CameraForward, ToTarget);

    // We'll use this to flag that the target is behind the camera
    bool bIsBehind = (Dot < 0.f);

    FVector2D ViewportSize(0, 0);
    if (GEngine && GEngine->GameViewport)
        GEngine->GameViewport->GetViewportSize(ViewportSize);

    FVector2D ScreenPos;
    ProjectWorldLocationToScreen(DetectorLocation, ScreenPos, false);

    // We'll treat negative z in camera space as "behind"
    bool bIsOnScreen = false;
    const float Margin = 1.0f;
    if (ScreenPos.X >= Margin && ScreenPos.X <= ViewportSize.X - Margin &&
        ScreenPos.Y >= Margin && ScreenPos.Y <= ViewportSize.Y - Margin && !bIsBehind)
    {
        bIsOnScreen = true;
    }
    else
    {
        // If behind, flip to front of camera for screen placement, then clamp to border.
        if (bIsBehind && ViewportSize.X > 1 && ViewportSize.Y > 1)
        {
            // This maps the target to the closest border from the center using the -camera->target direction in screen space
            FVector CamToTargetWS = (DetectorLocation - CameraLoc).GetSafeNormal();

            // Direction from camera, in camera space
            FVector CamSpaceForward = CameraRot.Vector();
            FVector RightVector = FRotationMatrix(CameraRot).GetUnitAxis(EAxis::Y);
            FVector UpVector = FRotationMatrix(CameraRot).GetUnitAxis(EAxis::Z);

            float X = FVector::DotProduct(RightVector, CamToTargetWS);
            float Y = FVector::DotProduct(UpVector, CamToTargetWS);

            FVector2D ToTarget2D(X, Y);

            // Invert Y for screen-space (UE screen Y+ is down)
            ToTarget2D.Y *= -1.f;

            // Normalize to get direction; fallback to bottom if zero
            if (ToTarget2D.IsNearlyZero())
                ToTarget2D = FVector2D(0.f, 1.f);

            ToTarget2D.Normalize();

            // Find how far we can go before reaching screen edge with margin
            FVector2D Center(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

            // "Box cast" border
            float ScaleX = (ToTarget2D.X > 0.f) ? (ViewportSize.X - Center.X - Margin) / FMath::Max(ToTarget2D.X, 0.0001f)
                                                : (0.f + Margin - Center.X) / FMath::Min(ToTarget2D.X, -0.0001f);
            float ScaleY = (ToTarget2D.Y > 0.f) ? (ViewportSize.Y - Center.Y - Margin) / FMath::Max(ToTarget2D.Y, 0.0001f)
                                                : (0.f + Margin - Center.Y) / FMath::Min(ToTarget2D.Y, -0.0001f);

            float Scale = FMath::Min(FMath::Abs(ScaleX), FMath::Abs(ScaleY));
            ScreenPos = Center + ToTarget2D * Scale;

            // Clamp, just in case
            ScreenPos.X = FMath::Clamp(ScreenPos.X, Margin, ViewportSize.X - Margin);
            ScreenPos.Y = FMath::Clamp(ScreenPos.Y, Margin, ViewportSize.Y - Margin);

            bIsOnScreen = false;
        }
    }

    CharacterHUD->CharacterOverlay->UpdateDetectionWidget(Detector, Progress, bIsLocked, ScreenPos, bIsOnScreen);
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
            CharacterHUD->CharacterOverlay->SetStatusTextWithColor(TEXT("Illegal"), FLinearColor::Red);
        }
        else
        {
            CharacterHUD->CharacterOverlay->SetStatusTextWithColor(TEXT(""), FLinearColor::White);
        }
    }
}

void ADefaultPlayerController::ServerExecuteAction_Implementation(UObject* Target, const FServerActionPayload& Payload)
{
    UE_LOG(LogTemp, Warning, TEXT("ServerExecuteAction_Implementation called! Target=%s"), *GetNameSafe(Target));
    if (Target && Target->GetClass()->ImplementsInterface(UServerActionInterface::StaticClass()))
    {
        UE_LOG(LogTemp, Warning, TEXT("Target implements IServerActionInterface, invoking ExecuteServerAction..."));
        IServerActionInterface::Execute_ExecuteServerAction(Target, Payload);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Target does NOT implement IServerActionInterface!"));
    }
}

