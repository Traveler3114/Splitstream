#include "LockPickComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

ULockPickComponent::ULockPickComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);

    if (Pins.Num() == 0)
    {
        Pins.SetNum(3);
        Pins[0].SweetSpotAngle = 30.f;
        Pins[1].SweetSpotAngle = 120.f;
        Pins[2].SweetSpotAngle = 270.f;
        Pins[0].Tolerance = 10.f;
        Pins[1].Tolerance = 10.f;
        Pins[2].Tolerance = 10.f;
    }
    PinSetStates.Init(false, Pins.Num());
    CurrentPinIndex = 0;
    bUnlocked = false;
    bPickingInProgress = false;
}

void ULockPickComponent::BeginPlay()
{
    Super::BeginPlay();
    PinSetStates.Init(false, Pins.Num());
    UE_LOG(LogTemp, Warning, TEXT("LockPickComponent::BeginPlay [%s] [Owner=%s] [HasAuth=%d]"), *GetName(), GetOwner() ? *GetOwner()->GetName() : TEXT("None"), GetOwner() ? GetOwner()->HasAuthority() : -1);
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan, FString::Printf(TEXT("LPC::BeginPlay [%s] Auth=%d"), *GetName(), GetOwner() ? GetOwner()->HasAuthority() : -1));
}

void ULockPickComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ULockPickComponent, CurrentPinIndex);
    DOREPLIFETIME(ULockPickComponent, PinSetStates);
    DOREPLIFETIME(ULockPickComponent, bUnlocked);
    DOREPLIFETIME(ULockPickComponent, bPickingInProgress);
}

void ULockPickComponent::ResetLock()
{
    UE_LOG(LogTemp, Warning, TEXT("LockPickComponent::ResetLock [%s]"), *GetName());
    PinSetStates.Init(false, Pins.Num());
    CurrentPinIndex = 0;
    bUnlocked = false;
}

void ULockPickComponent::StartLockPicking()
{
    UE_LOG(LogTemp, Warning, TEXT("LockPickComponent::StartLockPicking [%s]"), *GetName());
    ResetLock();
    bPickingInProgress = true;
}

void ULockPickComponent::EndLockPicking()
{
    UE_LOG(LogTemp, Warning, TEXT("LockPickComponent::EndLockPicking [%s]"), *GetName());
    bPickingInProgress = false;
}

bool ULockPickComponent::GetCurrentPinData(float& OutSweetSpotAngle, float& OutTolerance) const
{
    if (Pins.IsValidIndex(CurrentPinIndex))
    {
        OutSweetSpotAngle = Pins[CurrentPinIndex].SweetSpotAngle;
        OutTolerance = Pins[CurrentPinIndex].Tolerance;
        return true;
    }
    OutSweetSpotAngle = 0.f;
    OutTolerance = 0.f;
    return false;
}

bool ULockPickComponent::TrySetCurrentPin(float InputAngle)
{
    UE_LOG(LogTemp, Warning, TEXT("LPC::TrySetCurrentPin [%s] Angle=%.2f CurIdx=%d Progress=%d Unlocked=%d"), *GetName(), InputAngle, CurrentPinIndex, bPickingInProgress, bUnlocked);
    if (!bPickingInProgress || bUnlocked) return false;
    if (!Pins.IsValidIndex(CurrentPinIndex)) return false;

    float SweetSpot = NormalizeAngle(Pins[CurrentPinIndex].SweetSpotAngle);
    float Tolerance = Pins[CurrentPinIndex].Tolerance;
    float Input = NormalizeAngle(InputAngle);

    float Diff = FMath::Abs(SweetSpot - Input);
    if (Diff > 180.f) Diff = 360.f - Diff;

    bool bInTolerance = (Diff <= Tolerance);
    UE_LOG(LogTemp, Warning, TEXT("LPC::TrySetCurrentPin Sweet=%.2f Tol=%.2f Input=%.2f Diff=%.2f InTol=%d"), SweetSpot, Tolerance, Input, Diff, bInTolerance);

    if (bInTolerance)
    {
        PinSetStates[CurrentPinIndex] = true;
        return true;
    }
    return false;
}

bool ULockPickComponent::AdvancePin()
{
    UE_LOG(LogTemp, Warning, TEXT("LPC::AdvancePin [%s] CurIdx=%d"), *GetName(), CurrentPinIndex);
    if (Pins.IsValidIndex(CurrentPinIndex))
    {
        PinSetStates[CurrentPinIndex] = true;
        ++CurrentPinIndex;
        if (CurrentPinIndex >= Pins.Num())
        {
            bUnlocked = true;
            bPickingInProgress = false;
            UE_LOG(LogTemp, Error, TEXT("LPC::AdvancePin UNLOCKED [%s]"), *GetName());
            if (GEngine)
                GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("LPC::AdvancePin UNLOCKED [%s]"), *GetName()));
            OnUnlock.Broadcast();
            OnRep_Unlocked();
            return true;
        }
        return false;
    }
    return false;
}

float ULockPickComponent::NormalizeAngle(float Angle) const
{
    float A = FMath::Fmod(Angle, 360.f);
    if (A < 0.f) A += 360.f;
    return A;
}

void ULockPickComponent::OnRep_Unlocked()
{
    UE_LOG(LogTemp, Warning, TEXT("LPC::OnRep_Unlocked [%s] bUnlocked=%d [Owner=%s]"), *GetName(), bUnlocked, GetOwner() ? *GetOwner()->GetName() : TEXT("None"));
    if (bUnlocked)
    {
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("LPC::OnRep_Unlocked [%s]"), *GetName()));
        OnUnlock.Broadcast();
    }
}

void ULockPickComponent::ServerTrySetPin_Implementation(float InputAngle)
{
    UE_LOG(LogTemp, Warning, TEXT("LPC::ServerTrySetPin_Implementation [%s] Angle=%.2f [Owner=%s]"), *GetName(), InputAngle, GetOwner() ? *GetOwner()->GetName() : TEXT("None"));
    if (TrySetCurrentPin(InputAngle))
    {
        if (AdvancePin())
        {
            EndLockPicking();
            OnUnlock.Broadcast();
        }
    }
}