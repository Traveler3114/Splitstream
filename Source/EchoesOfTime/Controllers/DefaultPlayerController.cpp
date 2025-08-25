#include "DefaultPlayerController.h"
#include "LockPickingSystem/LockPickComponent.h"
#include "Characters/DefaultCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

#include "Widgets/HUD/CharacterHUD.h"

ADefaultPlayerController::ADefaultPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ADefaultPlayerController::BeginPlay()
{
    Super::BeginPlay();
    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;
    if (CharacterHUD)
    {
        if (CharacterHUD->CharacterOverlay == nullptr) CharacterHUD->AddCharacterOverlay();
    }
    CurrentInputMode = EPlayerInputMode::Gameplay;
}

void ADefaultPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Only add lockpick bindings once, they will only be active when minigame is running
    InputComponent->BindAxisKey(EKeys::MouseX, this, &ADefaultPlayerController::OnLockPickInputX);
    InputComponent->BindAxisKey(EKeys::MouseY, this, &ADefaultPlayerController::OnLockPickInputY);
    InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &ADefaultPlayerController::OnLockPickConfirm);
    InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ADefaultPlayerController::OnLockPickCancel);
}

void ADefaultPlayerController::SetPlayerInputMode(EPlayerInputMode NewMode)
{
    if (CurrentInputMode == NewMode) return;

    if (NewMode == EPlayerInputMode::LockPicking)
    {
        SetCharacterInputEnabled(false);
        bShowMouseCursor = true;
    }
    else // Gameplay
    {
        SetCharacterInputEnabled(true);
        bShowMouseCursor = false;
    }

    CurrentInputMode = NewMode;
}

void ADefaultPlayerController::SetCharacterInputEnabled(bool bEnabled)
{
    APawn* MyPawn = GetPawn();
    if (ADefaultCharacter* Char = Cast<ADefaultCharacter>(MyPawn))
    {
        if (bEnabled)
        {
            Char->EnableInput(this);
            Char->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        }
        else
        {
            Char->DisableInput(this);
            Char->GetCharacterMovement()->DisableMovement();
        }
    }
}

void ADefaultPlayerController::StartLockPickMinigame(ULockPickComponent* LockComp)
{
    if (bIsLockPicking || !LockComp) return;

    ActiveLockPickComp = LockComp;
    LockPickInputVector = FVector2D::ZeroVector;
    LockPickDialAngle = 0.0f;
    bIsLockPicking = true;
    LockComp->StartLockPicking();

    SetPlayerInputMode(EPlayerInputMode::LockPicking);

    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Lockpick minigame started!"));
}

void ADefaultPlayerController::EndLockPickMinigame(bool bSuccess)
{
    bIsLockPicking = false;
    if (ActiveLockPickComp)
    {
        ActiveLockPickComp->EndLockPicking();
        ActiveLockPickComp = nullptr;
    }

    SetPlayerInputMode(EPlayerInputMode::Gameplay);

    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, bSuccess ? FColor::Green : FColor::Red, bSuccess ? TEXT("Lock opened!") : TEXT("Lockpick canceled!"));
}

void ADefaultPlayerController::OnLockPickInputX(float Axis)
{
    if (bIsLockPicking)
    {
        LockPickInputVector.X += Axis * 2.0f;
        LockPickInputVector = LockPickInputVector.GetClampedToMaxSize(150.f);
        LockPickDialAngle = FMath::Atan2(LockPickInputVector.Y, LockPickInputVector.X) * (180.f / PI);
        if (LockPickDialAngle < 0) LockPickDialAngle += 360.f;
    }
}

void ADefaultPlayerController::OnLockPickInputY(float Axis)
{
    if (bIsLockPicking)
    {
        LockPickInputVector.Y += Axis * 2.0f;
        LockPickInputVector = LockPickInputVector.GetClampedToMaxSize(150.f);
        LockPickDialAngle = FMath::Atan2(LockPickInputVector.Y, LockPickInputVector.X) * (180.f / PI);
        if (LockPickDialAngle < 0) LockPickDialAngle += 360.f;
    }
}

void ADefaultPlayerController::OnLockPickConfirm()
{
    if (bIsLockPicking && ActiveLockPickComp)
    {
        ActiveLockPickComp->ServerTrySetPin(LockPickDialAngle);
    }
}

void ADefaultPlayerController::OnLockPickCancel()
{
    if (bIsLockPicking)
    {
        EndLockPickMinigame(false);
    }
}

void ADefaultPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bIsLockPicking && ActiveLockPickComp)
    {
        UpdateLockPickDebug();

        if (ActiveLockPickComp->bUnlocked)
        {
            EndLockPickMinigame(true);
        }
        else if (!ActiveLockPickComp->bPickingInProgress)
        {
            EndLockPickMinigame(false);
        }
    }
}

void ADefaultPlayerController::UpdateLockPickDebug()
{
    if (!ActiveLockPickComp) return;
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    float SweetSpot = 0, Tolerance = 0;
    int32 PinNum = ActiveLockPickComp->CurrentPinIndex + 1;
    int32 PinCount = ActiveLockPickComp->GetPinCount();
    bool bValid = ActiveLockPickComp->GetCurrentPinData(SweetSpot, Tolerance);

    FString Info = FString::Printf(TEXT("Lockpick: Pin %d/%d | Sweet: %.1f | Tol: %.1f | Angle: %.1f"),
        PinNum, PinCount, SweetSpot, Tolerance, LockPickDialAngle);
    if (GEngine) GEngine->AddOnScreenDebugMessage(1, 0.01f, FColor::Yellow, Info);

    FVector Center = MyPawn->GetActorLocation() + FVector(0, 0, 100);
    float Radius = 60.f;

    DrawDebugCircle(GetWorld(), Center, Radius, 32, FColor::White, false, 0.f, 0, 2.f, FVector(1, 0, 0), FVector(0, 1, 0), false);

    if (bValid)
    {
        float Start = SweetSpot - Tolerance;
        float End = SweetSpot + Tolerance;
        for (float A = Start; A <= End; A += 4)
        {
            float Rad = FMath::DegreesToRadians(A);
            FVector P = Center + FVector(FMath::Cos(Rad), FMath::Sin(Rad), 0) * Radius;
            DrawDebugPoint(GetWorld(), P, 10, FColor::Yellow, false, 0.01f);
        }
    }

    float Rad = FMath::DegreesToRadians(LockPickDialAngle);
    FVector End = Center + FVector(FMath::Cos(Rad), FMath::Sin(Rad), 0) * Radius;
    DrawDebugLine(GetWorld(), Center, End, FColor::Red, false, 0.01f, 0, 4);
}