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
    PreAlarmSoonestInstigator = nullptr;
    PreAlarmInstigatorsInfo.Empty();
    PreAlarmDuration = 3.f;
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

    // Cancel any pre-alarm that might be active
    if (bPreAlarmActive)
    {
        bPreAlarmActive = false;
        PreAlarmEndTime = 0.f;
        PreAlarmSoonestInstigator = nullptr;
        PreAlarmInstigatorsInfo.Empty();
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

void ADefaultGameState::StartPreAlarm(AActor* InPreAlarmInstigator, float Duration, ETimelineEra Era)
{
    if (!HasAuthority()) return;
    if (bAlarmActive) return;
    if (!InPreAlarmInstigator) return;

    const float ServerNow = GetWorld()->GetTimeSeconds();
    float EndTime = ServerNow + Duration;

    bool bFound = false;
    for (FPreAlarmInstigatorInfo& Info : PreAlarmInstigatorsInfo) {
        if (Info.Instigator == InPreAlarmInstigator) {
            if (EndTime < Info.ETA)
                Info.ETA = EndTime;
            Info.Era = Era;
            bFound = true;
            break;
        }
    }
    if (!bFound)
        PreAlarmInstigatorsInfo.Add(FPreAlarmInstigatorInfo(InPreAlarmInstigator, EndTime, Era));

    UpdatePreAlarmSoonestInstigator();

    if (!bPreAlarmActive) {
        PreAlarmEndTime = EndTime;
        bPreAlarmActive = true;
        ETimelineEra SoonestEra = Era;
        if (PreAlarmSoonestInstigator)
            for (const FPreAlarmInstigatorInfo& Info : PreAlarmInstigatorsInfo)
                if (Info.Instigator == PreAlarmSoonestInstigator)
                    SoonestEra = Info.Era;
        OnPreAlarmStarted.Broadcast(PreAlarmEndTime, PreAlarmSoonestInstigator, SoonestEra);
    }
}

void ADefaultGameState::RemovePreAlarmInstigator(AActor* InToRemoveInstigator)
{
    if (!HasAuthority()) return;
    if (!InToRemoveInstigator) return;

    PreAlarmInstigatorsInfo.RemoveAll([InToRemoveInstigator](const FPreAlarmInstigatorInfo& Info) {
        return Info.Instigator == InToRemoveInstigator;
        });

    UpdatePreAlarmSoonestInstigator();

    // If none left, cancel pre-alarm
    if (PreAlarmInstigatorsInfo.Num() == 0)
    {
        bPreAlarmActive = false;
        PreAlarmEndTime = 0.f;
        PreAlarmSoonestInstigator = nullptr;
        OnPreAlarmCanceled.Broadcast();
    }
    else
    {
        // Sync pre-alarm endtime and broadcast the new soonest
        PreAlarmEndTime = 0.f;
        ETimelineEra SoonestEra = ETimelineEra::Past;
        for (const FPreAlarmInstigatorInfo& Info : PreAlarmInstigatorsInfo)
            if (PreAlarmEndTime == 0.f || Info.ETA < PreAlarmEndTime)
            {
                PreAlarmEndTime = Info.ETA;
                SoonestEra = Info.Era;
            }
        OnPreAlarmStarted.Broadcast(PreAlarmEndTime, PreAlarmSoonestInstigator, SoonestEra);
    }
}


void ADefaultGameState::CancelPreAlarm(AActor* InCancelingInstigator)
{
    if (!HasAuthority())
        return;
    if (!bPreAlarmActive)
        return;

    if (InCancelingInstigator)
    {
        RemovePreAlarmInstigator(InCancelingInstigator);
    }
    else
    {
        PreAlarmInstigatorsInfo.Empty();
        bPreAlarmActive = false;
        PreAlarmEndTime = 0.f;
        PreAlarmSoonestInstigator = nullptr;
        OnPreAlarmCanceled.Broadcast();
    }
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
    // Find soonest again clientside by looking through PreAlarmInstigatorsInfo
    AActor* Soonest = nullptr;
    float SoonestETA = 0.f;
    ETimelineEra SoonestEra = ETimelineEra::Past;
    for (const FPreAlarmInstigatorInfo& Info : PreAlarmInstigatorsInfo)
    {
        if (Soonest == nullptr || Info.ETA < SoonestETA)
        {
            Soonest = Info.Instigator;
            SoonestETA = Info.ETA;
            SoonestEra = Info.Era;
        }
    }
    OnPreAlarmStarted.Broadcast(PreAlarmEndTime, Soonest, SoonestEra);
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
    DOREPLIFETIME(ADefaultGameState, PreAlarmInstigatorsInfo);
    DOREPLIFETIME(ADefaultGameState, PreAlarmSoonestInstigator);

    DOREPLIFETIME(ADefaultGameState, CurrentMoneyCollected);
    DOREPLIFETIME(ADefaultGameState, GuardRepairCountdowns);
}

void ADefaultGameState::UpdatePreAlarmSoonestInstigator()
{
    PreAlarmSoonestInstigator = nullptr;
    float SoonestETA = 0.f;
    float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
    for (const FPreAlarmInstigatorInfo& Info : PreAlarmInstigatorsInfo)
    {
        if (Info.Instigator)
        {
            float ThisETA = Info.ETA;
            // ETA should be in the future, but just in case
            if (ThisETA < Now)
                continue;
            if (!PreAlarmSoonestInstigator || ThisETA < SoonestETA)
            {
                PreAlarmSoonestInstigator = Info.Instigator;
                SoonestETA = ThisETA;
            }
        }
    }
}
