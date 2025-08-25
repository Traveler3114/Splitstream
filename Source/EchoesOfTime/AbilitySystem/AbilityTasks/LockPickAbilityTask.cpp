#include "LockPickAbilityTask.h"
#include "LockPickingSystem/LockPickComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "InputCoreTypes.h"
#include "DrawDebugHelpers.h"
#include "Controllers/DefaultPlayerController.h"

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
        UE_LOG(LogTemp, Error, TEXT("LockPickAbilityTask::Activate: LockComp is null!"));
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("LockPickTask: LockComp is NULL!"));
        FinishTask(false);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("LockPickAbilityTask::Activate [%s] [HasAuth=%d]"), *LockComp->GetName(), GetAvatarActor()->HasAuthority());
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Lockpick task started!"));

    // Only the server should reset and start lockpicking!
    if (GetAvatarActor()->HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("LockPickAbilityTask::Activate: Calling StartLockPicking on server!"));
        LockComp->StartLockPicking();
    }

    LockPickInputVector = FVector2D::ZeroVector;
    LockPickDialAngle = 0.0f;
    bIsLockPicking = true;

    BindInput();

    bTickingTask = true; // Enable ticking
}

void ULockPickAbilityTask::OnDestroy(bool bInOwnerFinished)
{
    UE_LOG(LogTemp, Warning, TEXT("LockPickAbilityTask::OnDestroy"));
    UnbindInput();
    bTickingTask = false; // Disable ticking
    bIsLockPicking = false;
    Super::OnDestroy(bInOwnerFinished);
}

void ULockPickAbilityTask::TickTask(float DeltaTime)
{
    if (!bIsLockPicking || !LockComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("LockPickAbilityTask::TickTask: bIsLockPicking=%d, LockComp=%p"), bIsLockPicking, LockComp);
        FinishTask(false);
        return;
    }

    UpdateLockPickDebug();

    // Only care about server state!
    //UE_LOG(LogTemp, Warning, TEXT("LockPickAbilityTask::TickTask: bUnlocked=%d bPickingInProgress=%d [LockComp=%p]"), LockComp->bUnlocked, LockComp->bPickingInProgress, LockComp);

    if (LockComp->bUnlocked)
    {
        //UE_LOG(LogTemp, Warning, TEXT("LockPickAbilityTask::TickTask: LOCK OPENED!"));
        //if (GEngine)
        //    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Lock opened!"));
        FinishTask(true);
    }
    else if (!LockComp->bPickingInProgress)
    {
        //UE_LOG(LogTemp, Warning, TEXT("LockPickAbilityTask::TickTask: Lockpick canceled!"));
        //if (GEngine)
        //    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Lockpick canceled!"));
        FinishTask(false);
    }
}

void ULockPickAbilityTask::BindInput()
{
    APawn* Pawn = Cast<APawn>(GetAvatarActor());
    if (!Pawn || !Pawn->InputComponent) {
        UE_LOG(LogTemp, Error, TEXT("LockPickAbilityTask::BindInput: Pawn or InputComponent is null!"));
        return;
    }
    Pawn->InputComponent->BindAxisKey(EKeys::MouseX, this, &ULockPickAbilityTask::OnMouseX);
    Pawn->InputComponent->BindAxisKey(EKeys::MouseY, this, &ULockPickAbilityTask::OnMouseY);
    Pawn->InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &ULockPickAbilityTask::OnConfirm);
    Pawn->InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ULockPickAbilityTask::OnCancel);

    UE_LOG(LogTemp, Warning, TEXT("LockPickAbilityTask::BindInput: Bound input to pawn %s"), *Pawn->GetName());
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

//void ULockPickAbilityTask::OnConfirm()
//{
//    UE_LOG(LogTemp, Warning, TEXT("LockPickAbilityTask::OnConfirm: Sending ServerConfirmPin with Angle=%.2f"), LockPickDialAngle);
//    if (GEngine)
//        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("LockPickTask: Confirm (%.2f)"), LockPickDialAngle));
//    ServerConfirmPin(LockPickDialAngle);
//}

void ULockPickAbilityTask::OnConfirm()
{
    APawn* Pawn = Cast<APawn>(GetAvatarActor());
    if (!Pawn) return;
    APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
    if (!PC) return;

    // If using a custom controller class, cast to ADefaultPlayerController!
    ADefaultPlayerController* MyPC = Cast<ADefaultPlayerController>(PC);
    if (!MyPC || !LockComp) return;

    MyPC->ServerLockPickConfirm(LockComp->GetOwner(), LockPickDialAngle);
}

void ULockPickAbilityTask::ServerConfirmPin_Implementation(float Angle)
{
    UE_LOG(LogTemp, Error, TEXT("LockPickAbilityTask::ServerConfirmPin_Implementation: Angle=%.2f [HasAuth=%d] LockComp=%p"), Angle, GetAvatarActor()->HasAuthority(), LockComp);
    if (!LockComp || !bIsLockPicking) {
        UE_LOG(LogTemp, Error, TEXT("LockPickAbilityTask::ServerConfirmPin_Implementation: LockComp is null or not picking!"));
        return;
    }

    bool bCorrect = LockComp->TrySetCurrentPin(Angle);
    UE_LOG(LogTemp, Warning, TEXT("LockPickAbilityTask::ServerConfirmPin_Implementation: TrySetCurrentPin returned %d"), bCorrect);
    if (bCorrect)
    {
        bool bUnlocked = LockComp->AdvancePin();
        UE_LOG(LogTemp, Warning, TEXT("LockPickAbilityTask::ServerConfirmPin_Implementation: AdvancePin returned %d"), bUnlocked);
        if (bUnlocked)
        {
            LockComp->EndLockPicking();
        }
    }
}

void ULockPickAbilityTask::OnCancel()
{
    UE_LOG(LogTemp, Warning, TEXT("LockPickAbilityTask::OnCancel"));
    if (!bIsLockPicking) return;
    bIsLockPicking = false;

    // Only the server should end picking for everyone
    if (GetAvatarActor()->HasAuthority())
    {
        if (LockComp) LockComp->EndLockPicking();
    }
    OnFinished.Broadcast(false);
    EndTask();
}

void ULockPickAbilityTask::FinishTask(bool bSuccess)
{
    UE_LOG(LogTemp, Warning, TEXT("LockPickAbilityTask::FinishTask: bSuccess=%d"), bSuccess);
    bIsLockPicking = false;
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
    FVector EndPt = Center + FVector(FMath::Cos(Rad), FMath::Sin(Rad), 0) * Radius;
    DrawDebugLine(Pawn->GetWorld(), Center, EndPt, FColor::Red, false, 0.01f, 0, 4);
}