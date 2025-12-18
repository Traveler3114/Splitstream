// DefaultGameState.cpp

#include "DefaultGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

ADefaultGameState::ADefaultGameState()
{
    AlarmEndTime = 0.f;
    bAlarmActive = false;
    AlarmDuration = 5.f;
    AlarmInstigator = nullptr;

    PreAlarmEndTime = 0.f;
    bPreAlarmActive = false;
    PreAlarmInstigator = nullptr;
    PreAlarmDuration = 3.f;
}

void ADefaultGameState::StartGuardRepairCountdown(ARepairableBase* Repairable, float Duration)
{
    if (!HasAuthority()) return;
    bool bFound = false;
    for (FGuardRepairCountdown& Entry : GuardRepairCountdowns)
    {
        if (Entry.TargetActor == Repairable) { Entry.ETA = Duration; bFound = true; break; }
    }
    if (!bFound)
    {
        GuardRepairCountdowns.Add({ Repairable, Duration });
    }
    OnGuardRepairETAStarted.Broadcast(Repairable, Duration);
}

void ADefaultGameState::StartAlarm(AActor* InAlarmInstigator)
{
    if (!HasAuthority())
        return;

    if (bAlarmActive)
        return;

    const float ServerNow = GetWorld()->GetTimeSeconds();
    AlarmEndTime = ServerNow + AlarmDuration;
    bAlarmActive = true;
    AlarmInstigator = InAlarmInstigator;

    // Cancel any pre-alarm that might be active
    if (bPreAlarmActive)
    {
        bPreAlarmActive = false;
        PreAlarmEndTime = 0.f;
        PreAlarmInstigator = nullptr;
        OnPreAlarmCanceled.Broadcast();
    }

    OnAlarmStarted.Broadcast(AlarmEndTime);
}

void ADefaultGameState::CancelAlarm(AActor* InAlarmInstigator)
{
    if (!HasAuthority())
        return;

    if (!bAlarmActive)
        return;

    if (InAlarmInstigator && AlarmInstigator && InAlarmInstigator != AlarmInstigator)
        return;

    bAlarmActive = false;
    AlarmEndTime = 0.f;
    AlarmInstigator = nullptr;
    OnAlarmCanceled.Broadcast();
}

float ADefaultGameState::GetRemainingAlarmTime() const
{
    if (!bAlarmActive)
        return 0.f;
    const float Now = GetWorld()->GetTimeSeconds();
    return FMath::Max(0.f, AlarmEndTime - Now);
}

void ADefaultGameState::RequestRestart()
{
    OnRestartRequested.Broadcast();
}

void ADefaultGameState::StartPreAlarm(AActor* InPreAlarmInstigator, float Duration)
{
    if (!HasAuthority())
        return;
    if (bAlarmActive)
        return;
    if (bPreAlarmActive)
        return;

    const float ServerNow = GetWorld()->GetTimeSeconds();
    PreAlarmEndTime = ServerNow + Duration;
    bPreAlarmActive = true;
    PreAlarmInstigator = InPreAlarmInstigator;
    OnPreAlarmStarted.Broadcast(PreAlarmEndTime, PreAlarmInstigator);
}

void ADefaultGameState::CancelPreAlarm(AActor* InPreAlarmInstigator)
{
    if (!HasAuthority())
        return;
    if (!bPreAlarmActive)
        return;
    if (InPreAlarmInstigator && PreAlarmInstigator && InPreAlarmInstigator != PreAlarmInstigator)
        return;

    bPreAlarmActive = false;
    PreAlarmEndTime = 0.f;
    PreAlarmInstigator = nullptr;
    OnPreAlarmCanceled.Broadcast();
}

float ADefaultGameState::GetRemainingPreAlarmTime() const
{
    if (!bPreAlarmActive)
        return 0.f;
    const float Now = GetWorld()->GetTimeSeconds();
    return FMath::Max(0.f, PreAlarmEndTime - Now);
}

void ADefaultGameState::OnRep_AlarmStarted()
{
    OnAlarmStarted.Broadcast(AlarmEndTime);
}

void ADefaultGameState::OnRep_AlarmActive()
{
    if (!bAlarmActive)
        OnAlarmCanceled.Broadcast();
}

void ADefaultGameState::OnRep_PreAlarmStarted()
{
    OnPreAlarmStarted.Broadcast(PreAlarmEndTime, PreAlarmInstigator);
}

void ADefaultGameState::OnRep_PreAlarmActive()
{
    if (!bPreAlarmActive)
        OnPreAlarmCanceled.Broadcast();
}

void ADefaultGameState::AddCollectedMoney(int32 Amount)
{
    if (!HasAuthority()) return;
    CurrentMoneyCollected += Amount;
    CurrentMoneyCollected = FMath::Min(CurrentMoneyCollected, TargetMoneyAmount);
    OnMoneyCollectedChanged.Broadcast(CurrentMoneyCollected, TargetMoneyAmount);
}

void ADefaultGameState::OnRep_CurrentMoneyCollected()
{
    OnMoneyCollectedChanged.Broadcast(CurrentMoneyCollected, TargetMoneyAmount);
}

void ADefaultGameState::OnRep_GuardRepairCountdowns()
{
    for (const FGuardRepairCountdown& Entry : GuardRepairCountdowns)
        OnGuardRepairETAStarted.Broadcast(Entry.TargetActor, Entry.ETA);
}

void ADefaultGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ADefaultGameState, AlarmEndTime);
    DOREPLIFETIME(ADefaultGameState, bAlarmActive);
    DOREPLIFETIME(ADefaultGameState, AlarmInstigator);

    DOREPLIFETIME(ADefaultGameState, PreAlarmEndTime);
    DOREPLIFETIME(ADefaultGameState, bPreAlarmActive);
    DOREPLIFETIME(ADefaultGameState, PreAlarmInstigator);

    DOREPLIFETIME(ADefaultGameState, CurrentMoneyCollected);
    DOREPLIFETIME(ADefaultGameState, GuardRepairCountdowns);
}