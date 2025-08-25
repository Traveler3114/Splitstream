#include "LockPickComponent.h"
#include "Net/UnrealNetwork.h"

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
    PinSetStates.Init(false, Pins.Num());
    CurrentPinIndex = 0;
    bUnlocked = false;
}

void ULockPickComponent::StartLockPicking()
{
    ResetLock();
    bPickingInProgress = true;
}

void ULockPickComponent::EndLockPicking()
{
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
    if (!bPickingInProgress || bUnlocked) return false;
    if (!Pins.IsValidIndex(CurrentPinIndex)) return false;

    float SweetSpot = NormalizeAngle(Pins[CurrentPinIndex].SweetSpotAngle);
    float Tolerance = Pins[CurrentPinIndex].Tolerance;
    float Input = NormalizeAngle(InputAngle);

    float Diff = FMath::Abs(SweetSpot - Input);
    if (Diff > 180.f) Diff = 360.f - Diff;

    if (Diff <= Tolerance)
    {
        PinSetStates[CurrentPinIndex] = true;
        return true;
    }
    return false;
}

bool ULockPickComponent::AdvancePin()
{
    if (Pins.IsValidIndex(CurrentPinIndex))
    {
        PinSetStates[CurrentPinIndex] = true;
        ++CurrentPinIndex;
        if (CurrentPinIndex >= Pins.Num())
        {
            bUnlocked = true;
            bPickingInProgress = false;
            OnUnlock.Broadcast();
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


void ULockPickComponent::ServerTrySetPin_Implementation(float InputAngle)
{
    if (TrySetCurrentPin(InputAngle))
    {
        if (AdvancePin())
        {
            EndLockPicking();
            // (Optional: fire Blueprint event for unlock)
        }
    }
}