#include "LockPickComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/SplitstreamGameplayTags.h"

#if WITH_EDITOR
#include "Editor/UnrealEd/Public/Editor.h"
void ULockPickComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    GeneratePins();
    PinSetStates.Init(false, Pins.Num());
}
#endif

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
    if (GetOwner()->HasAuthority())
    {
        GeneratePins();
    }
    PinSetStates.Init(false, Pins.Num());
}

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

    // Use a GUID for the seed for maximum unpredictability
    FGuid Guid = FGuid::NewGuid();
    int32 Seed = HashCombine(GetTypeHash(Guid.A), GetTypeHash(Guid.B) ^ GetTypeHash(Guid.C) ^ GetTypeHash(Guid.D));
    FRandomStream Stream(Seed);

    for (int32 i = 0; i < NumPins; ++i)
    {
        float Angle = Stream.FRandRange(0.f, 360.f);
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
    DOREPLIFETIME(ULockPickComponent, Pins);
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

void ULockPickComponent::Interact(AActor* Interactor)
{
    if (bUnlocked || bPickingInProgress) {
        return;
    }
    if (IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(Interactor))
    {
        if (UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent())
        {
            FGameplayEventData EventData;
            EventData.Instigator = Interactor;
            EventData.OptionalObject = GetOwner();

            ASC->HandleGameplayEvent(
                TAG_Character_Ability_LockPick,
                &EventData
            );
        }
    }
}

void ULockPickComponent::CancelInteract(AActor* Interactor)
{
    EndLockPicking();
    if (Interactor)
    {
        // Get AbilitySystemComponent from the interactor
        if (IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(Interactor))
        {
            if (UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent())
            {
                FGameplayTagContainer CancelTags;
                CancelTags.AddTag(TAG_Character_Ability_LockPick);
                ASC->CancelAbilities(&CancelTags);
            }
        }
    }
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

float ULockPickComponent::GetPinAngleProximity(int32 PinIndex, float InputAngle) const
{
    if (!Pins.IsValidIndex(PinIndex)) return 0.f;
    float SweetSpot = NormalizeAngle(Pins[PinIndex].SweetSpotAngle);
    float Tolerance = Pins[PinIndex].Tolerance;
    float InputNorm = NormalizeAngle(InputAngle);

    float Diff = FMath::Abs(SweetSpot - InputNorm);
    if (Diff > 180.f) Diff = 360.f - Diff;

    return 1.0f - FMath::Clamp(Diff / Tolerance, 0.f, 1.f);
}