#include "LockPickComponent.h"
#include "Net/UnrealNetwork.h"

ULockPickComponent::ULockPickComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
    GeneratePins();
    PinSetStates.Init(false, Pins.Num());
    CurrentPinIndex = 0;
    bUnlocked = false;
    bPickingInProgress = false;
}

void ULockPickComponent::BeginPlay()
{
    Super::BeginPlay();
    // Only generate pins on the server for multiplayer safety
    if (GetOwner()->HasAuthority())
    {
        GeneratePins();
    }
    PinSetStates.Init(false, Pins.Num());
}

void ULockPickComponent::OnComponentCreated()
{
    Super::OnComponentCreated();
    // Only generate pins on the server for multiplayer safety
    if (GetOwner()->HasAuthority())
    {
        GeneratePins();
    }
    PinSetStates.Init(false, Pins.Num());
}

#if WITH_EDITOR
void ULockPickComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    GeneratePins();
    PinSetStates.Init(false, Pins.Num());
}
#endif

void ULockPickComponent::GeneratePins()
{
    Pins.Empty();
    int32 NumPins = FMath::Clamp(PinCount, 1, 5);

    float Tolerance = 10.f;
    switch (LockDifficulty)
    {
    case ELockDifficulty::Easy:   Tolerance = 10.f; break;
    case ELockDifficulty::Medium: Tolerance = 5.f; break;
    case ELockDifficulty::Hard:   Tolerance = 2.f;  break;
    default:                      Tolerance = 10.f; break;
    }

    // Seed the random stream with a truly random value for each session
    int32 Seed = FMath::Rand();
    FRandomStream Stream(Seed);

    for (int32 i = 0; i < NumPins; ++i)
    {
        float Angle = Stream.FRandRange(0.f, 360.f); // Truly random every time the level is loaded
        FLockPinData Pin;
        Pin.SweetSpotAngle = Angle;
        Pin.Tolerance = Tolerance;
        Pins.Add(Pin);
    }
    PinSetStates.Init(false, Pins.Num());
    CurrentPinIndex = 0;
    bUnlocked = false;
    bPickingInProgress = false;
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
    if (bPickingInProgress || bUnlocked) return;
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

    bool bInTolerance = (Diff <= Tolerance);
    if (bInTolerance)
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
    if (bUnlocked)
    {
        OnUnlock.Broadcast();
    }
}

void ULockPickComponent::ServerTrySetPin_Implementation(float InputAngle)
{
    if (TrySetCurrentPin(InputAngle))
    {
        if (AdvancePin())
        {
            EndLockPicking();
            OnUnlock.Broadcast();
        }
    }
}

float ULockPickComponent::GetPinAngleProximity(int32 PinIndex, float InputAngle) const
{
    if (!Pins.IsValidIndex(PinIndex)) return 0.f;
    float SweetSpot = NormalizeAngle(Pins[PinIndex].SweetSpotAngle);
    float Tolerance = Pins[PinIndex].Tolerance;
    float InputNorm = NormalizeAngle(InputAngle);

    float Diff = FMath::Abs(SweetSpot - InputNorm);
    if (Diff > 180.f) Diff = 360.f - Diff;

    return 1.0f - FMath::Clamp(Diff / Tolerance, 0.f, 1.f); // 1 when on sweet spot, 0 when outside tolerance
}