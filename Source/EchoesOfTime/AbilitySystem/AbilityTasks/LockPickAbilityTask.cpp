#include "LockPickAbilityTask.h"
#include "LockPickingSystem/LockPickComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "InputCoreTypes.h"
#include "DrawDebugHelpers.h"

ULockPickAbilityTask* ULockPickAbilityTask::StartLockPickTask(UGameplayAbility* OwningAbility, ULockPickComponent* InLockComp)
{
    ULockPickAbilityTask* Task = NewAbilityTask<ULockPickAbilityTask>(OwningAbility);
    Task->LockComp = InLockComp;
    return Task;
}

void ULockPickAbilityTask::Activate()
{
    if (!LockComp)
    {
        FinishTask(false);
        return;
    }

    LockComp->StartLockPicking();

    LockPickInputVector = FVector2D::ZeroVector;
    LockPickDialAngle = 0.0f;
    bIsLockPicking = true;

    BindInput();

    bTickingTask = true; // Enable ticking

    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Lockpick task started!"));
}

void ULockPickAbilityTask::OnDestroy(bool bInOwnerFinished)
{
    UnbindInput();
    bTickingTask = false; // Disable ticking
    bIsLockPicking = false;
    Super::OnDestroy(bInOwnerFinished);
}

void ULockPickAbilityTask::TickTask(float DeltaTime)
{
    if (!bIsLockPicking || !LockComp)
    {
        FinishTask(false);
        return;
    }

    UpdateLockPickDebug();

    if (LockComp->bUnlocked)
    {
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Lock opened!"));
        FinishTask(true);
    }
    else if (!LockComp->bPickingInProgress)
    {
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Lockpick canceled!"));
        FinishTask(false);
    }
}

void ULockPickAbilityTask::BindInput()
{
    APawn* Pawn = Cast<APawn>(GetAvatarActor());
    if (!Pawn || !Pawn->InputComponent) return;
    // Mouse X
    Pawn->InputComponent->BindAxisKey(EKeys::MouseX, this, &ULockPickAbilityTask::OnMouseX);
    // Mouse Y
    Pawn->InputComponent->BindAxisKey(EKeys::MouseY, this, &ULockPickAbilityTask::OnMouseY);
    // Confirm (Space)
    Pawn->InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &ULockPickAbilityTask::OnConfirm);
    // Cancel (Escape)
    Pawn->InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ULockPickAbilityTask::OnCancel);
}

void ULockPickAbilityTask::UnbindInput()
{
    // Unreal cleans up InputComponent on context switch, so explicit unbinding not required.
}

void ULockPickAbilityTask::OnMouseX(float Axis)
{
    if (!bIsLockPicking) return;
    LockPickInputVector.X += Axis * 2.0f;
    LockPickInputVector = LockPickInputVector.GetClampedToMaxSize(150.f);
    LockPickDialAngle = FMath::Atan2(LockPickInputVector.Y, LockPickInputVector.X) * (180.f / PI);
    if (LockPickDialAngle < 0) LockPickDialAngle += 360.f;
}

void ULockPickAbilityTask::OnMouseY(float Axis)
{
    if (!bIsLockPicking) return;
    LockPickInputVector.Y += Axis * 2.0f;
    LockPickInputVector = LockPickInputVector.GetClampedToMaxSize(150.f);
    LockPickDialAngle = FMath::Atan2(LockPickInputVector.Y, LockPickInputVector.X) * (180.f / PI);
    if (LockPickDialAngle < 0) LockPickDialAngle += 360.f;
}

void ULockPickAbilityTask::OnConfirm()
{
    if (!bIsLockPicking || !LockComp) return;
    LockComp->ServerTrySetPin(LockPickDialAngle);
}

void ULockPickAbilityTask::OnCancel()
{
    if (!bIsLockPicking) return;
    FinishTask(false);
}

void ULockPickAbilityTask::FinishTask(bool bSuccess)
{
    bIsLockPicking = false;
    if (LockComp)
    {
        LockComp->EndLockPicking();
    }
    OnFinished.Broadcast(bSuccess);
    EndTask();
}

void ULockPickAbilityTask::UpdateLockPickDebug()
{
    APawn* Pawn = Cast<APawn>(GetAvatarActor());
    if (!LockComp || !Pawn) return;

    float SweetSpot = 0, Tolerance = 0;
    int32 PinNum = LockComp->CurrentPinIndex + 1;
    int32 PinCount = LockComp->GetPinCount();
    bool bValid = LockComp->GetCurrentPinData(SweetSpot, Tolerance);

    FString Info = FString::Printf(TEXT("Lockpick: Pin %d/%d | Sweet: %.1f | Tol: %.1f | Angle: %.1f"),
        PinNum, PinCount, SweetSpot, Tolerance, LockPickDialAngle);
    if (GEngine) GEngine->AddOnScreenDebugMessage(1, 0.01f, FColor::Yellow, Info);

    FVector Center = Pawn->GetActorLocation() + FVector(0, 0, 100);
    float Radius = 60.f;

    DrawDebugCircle(Pawn->GetWorld(), Center, Radius, 32, FColor::White, false, 0.f, 0, 2.f, FVector(1, 0, 0), FVector(0, 1, 0), false);

    if (bValid)
    {
        float Start = SweetSpot - Tolerance;
        float End = SweetSpot + Tolerance;
        for (float A = Start; A <= End; A += 4)
        {
            float Rad = FMath::DegreesToRadians(A);
            FVector P = Center + FVector(FMath::Cos(Rad), FMath::Sin(Rad), 0) * Radius;
            DrawDebugPoint(Pawn->GetWorld(), P, 10, FColor::Yellow, false, 0.01f);
        }
    }

    float Rad = FMath::DegreesToRadians(LockPickDialAngle);
    FVector End = Center + FVector(FMath::Cos(Rad), FMath::Sin(Rad), 0) * Radius;
    DrawDebugLine(Pawn->GetWorld(), Center, End, FColor::Red, false, 0.01f, 0, 4);
}