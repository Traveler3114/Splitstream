#include "DefaultGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

ADefaultGameState::ADefaultGameState()
{
    AlarmEndTime = 0.f;
    bAlarmActive = false;
    AlarmDuration = 5.f;
    AlarmInstigator = nullptr;

    CurrentMoneyCollected = 0;
    TargetMoneyAmount = 50000;
}

void ADefaultGameState::StartGuardRepairCountdown(AActor* Repairable, float Duration)
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

void ADefaultGameState::StartAlarm(AActor* InAlarmInstigator, ETimelineEra Era)
{
    if (!HasAuthority())
        return;

    if (bAlarmActive)
        return;

    const float ServerNow = GetWorld()->GetTimeSeconds();
    AlarmEndTime = ServerNow + AlarmDuration;
    bAlarmActive = true;
    AlarmInstigator = InAlarmInstigator;
    AlarmEra = Era;

    // Optionally, cancel the corresponding era's prealarm state:
    FPerEraPreAlarmState& EraPreAlarm = GetEraPreAlarm(Era);
    EraPreAlarm.bActive = false;
    EraPreAlarm.EndTime = 0.f;
    EraPreAlarm.SoonestInstigator = nullptr;
    EraPreAlarm.InstigatorsInfo.Empty();

    OnAlarmStarted.Broadcast(AlarmEndTime, AlarmEra);
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

void ADefaultGameState::StartPreAlarm(AActor* InPreAlarmInstigator, float Duration, ETimelineEra Era)
{
    if (!HasAuthority()) return;
    if (!InPreAlarmInstigator) return;

    FPerEraPreAlarmState& State = GetEraPreAlarm(Era);

    const float ServerNow = GetWorld()->GetTimeSeconds();
    float EndTime = ServerNow + Duration;

    bool bFound = false;
    for (FPreAlarmInstigatorInfo& Info : State.InstigatorsInfo) {
        if (Info.Instigator == InPreAlarmInstigator) {
            if (EndTime < Info.ETA)
                Info.ETA = EndTime;
            Info.Era = Era;
            bFound = true;
            break;
        }
    }
    if (!bFound)
        State.InstigatorsInfo.Add(FPreAlarmInstigatorInfo(InPreAlarmInstigator, EndTime, Era));

    // Find soonest instigator for this era
    State.SoonestInstigator = nullptr;
    float SoonestETA = 0.f;
    for (const FPreAlarmInstigatorInfo& Info : State.InstigatorsInfo) {
        if (State.SoonestInstigator == nullptr || Info.ETA < SoonestETA)
        {
            State.SoonestInstigator = Info.Instigator;
            SoonestETA = Info.ETA;
        }
    }
    State.EndTime = SoonestETA;

    if (!State.bActive) {
        State.bActive = true;
        OnPreAlarmStarted.Broadcast(State.EndTime, State.SoonestInstigator, Era);
    }
}

void ADefaultGameState::RemovePreAlarmInstigator(AActor* InToRemoveInstigator, ETimelineEra Era)
{
    if (!HasAuthority()) return;
    if (!InToRemoveInstigator) return;

    FPerEraPreAlarmState& State = GetEraPreAlarm(Era);

    State.InstigatorsInfo.RemoveAll([InToRemoveInstigator](const FPreAlarmInstigatorInfo& Info) {
        return Info.Instigator == InToRemoveInstigator;
        });

    // Update soonest for this era
    State.SoonestInstigator = nullptr;
    float SoonestETA = 0.f;
    for (const FPreAlarmInstigatorInfo& Info : State.InstigatorsInfo)
    {
        if (State.SoonestInstigator == nullptr || Info.ETA < SoonestETA)
        {
            State.SoonestInstigator = Info.Instigator;
            SoonestETA = Info.ETA;
        }
    }
    State.EndTime = SoonestETA;

    if (State.InstigatorsInfo.Num() == 0)
    {
        State.bActive = false;
        State.EndTime = 0.f;
        State.SoonestInstigator = nullptr;
        OnPreAlarmCanceled.Broadcast();
    }
    else
    {
        OnPreAlarmStarted.Broadcast(State.EndTime, State.SoonestInstigator, Era);
    }
}

void ADefaultGameState::CancelPreAlarm(AActor* InCancelingInstigator, ETimelineEra Era)
{
    if (!HasAuthority())
        return;

    FPerEraPreAlarmState& State = GetEraPreAlarm(Era);

    if (!State.bActive)
        return;

    if (InCancelingInstigator)
    {
        RemovePreAlarmInstigator(InCancelingInstigator, Era);
    }
    else
    {
        State.InstigatorsInfo.Empty();
        State.bActive = false;
        State.EndTime = 0.f;
        State.SoonestInstigator = nullptr;
        OnPreAlarmCanceled.Broadcast();
    }
}

float ADefaultGameState::GetRemainingPreAlarmTime(ETimelineEra Era) const
{
    const FPerEraPreAlarmState& State = GetEraPreAlarm(Era);
    if (!State.bActive)
        return 0.f;
    const float Now = GetWorld()->GetTimeSeconds();
    return FMath::Max(0.f, State.EndTime - Now);
}

void ADefaultGameState::OnRep_AlarmStarted()
{
    OnAlarmStarted.Broadcast(AlarmEndTime, AlarmEra);
}

void ADefaultGameState::OnRep_AlarmActive()
{
    if (!bAlarmActive)
        OnAlarmCanceled.Broadcast();
}

void ADefaultGameState::OnRep_PreAlarmStarted()
{
    // Broadcast for Past prealarm if active
    if (PastPreAlarm.bActive)
        OnPreAlarmStarted.Broadcast(PastPreAlarm.EndTime, PastPreAlarm.SoonestInstigator, ETimelineEra::Past);

    // Broadcast for Future prealarm if active
    if (FuturePreAlarm.bActive)
        OnPreAlarmStarted.Broadcast(FuturePreAlarm.EndTime, FuturePreAlarm.SoonestInstigator, ETimelineEra::Future);
}
void ADefaultGameState::OnRep_PreAlarmActive()
{
    if (!PastPreAlarm.bActive && !FuturePreAlarm.bActive)
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
    DOREPLIFETIME(ADefaultGameState, AlarmEra);

    DOREPLIFETIME(ADefaultGameState, PastPreAlarm);
    DOREPLIFETIME(ADefaultGameState, FuturePreAlarm);
    DOREPLIFETIME(ADefaultGameState, GuardRepairCountdowns);
    DOREPLIFETIME(ADefaultGameState, CurrentMoneyCollected);
}