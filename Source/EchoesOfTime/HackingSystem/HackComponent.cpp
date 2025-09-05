#include "HackComponent.h"

UHackComponent::UHackComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bHackingInProgress = false;
    bHacked = false;
    HackDuration = 10.f;
    HackElapsed = 0.f;
}

void UHackComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UHackComponent::StartHacking(float Duration)
{
    if (bHackingInProgress || bHacked) return;
    bHackingInProgress = true;
    HackDuration = Duration;
    HackElapsed = 0.f;
    SetComponentTickEnabled(true);
}

void UHackComponent::CancelHacking()
{
    bHackingInProgress = false;
    SetComponentTickEnabled(false);
}

float UHackComponent::GetHackProgress() const
{
    if (!bHackingInProgress || HackDuration <= 0.f) return 0.f;
    return FMath::Clamp(HackElapsed / HackDuration, 0.f, 1.f);
}

void UHackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (bHackingInProgress)
    {
        HackElapsed += DeltaTime;
        if (HackElapsed >= HackDuration)
        {
            bHackingInProgress = false;
            bHacked = true;
            SetComponentTickEnabled(false);
            OnHackComplete.Broadcast();
        }
    }
}