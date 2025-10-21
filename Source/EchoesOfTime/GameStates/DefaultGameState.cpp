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
}

void ADefaultGameState::StartAlarm(AActor* InAlarmInstigator)
{
	// Only server should start the alarm
	if (!HasAuthority())
	{
		return;
	}

	// If already active, ignore (or you can choose to restart it)
	if (bAlarmActive)
	{
		return;
	}

	const float ServerNow = GetWorld()->GetTimeSeconds();
	AlarmEndTime = ServerNow + AlarmDuration;
	bAlarmActive = true;
	AlarmInstigator = InAlarmInstigator;

	// Immediately broadcast on server; replicated properties + OnRep will notify clients
	OnAlarmStarted.Broadcast(AlarmEndTime);
}

void ADefaultGameState::CancelAlarm(AActor* InAlarmInstigator)
{
	// Only server can cancel
	if (!HasAuthority())
	{
		return;
	}

	if (!bAlarmActive)
	{
		return;
	}

	// If an instigator is provided, only cancel if it matches the stored instigator.
	// This ensures e.g. killing a guard cancels alarms that guard started, but doesn't cancel laser-started alarms.
	if (InAlarmInstigator && AlarmInstigator && InAlarmInstigator != AlarmInstigator)
	{
		// instigator doesn't match, don't cancel
		return;
	}

	// Perform cancel
	bAlarmActive = false;
	AlarmEndTime = 0.f;
	AlarmInstigator = nullptr;

	// Notify server listeners immediately
	OnAlarmCanceled.Broadcast();
	// Clients will be notified via replication/OnRep_AlarmActive
}

float ADefaultGameState::GetRemainingAlarmTime() const
{
	if (!bAlarmActive)
	{
		return 0.f;
	}
	const float Now = GetWorld()->GetTimeSeconds();
	return FMath::Max(0.f, AlarmEndTime - Now);
}

void ADefaultGameState::RequestRestart()
{
	OnRestartRequested.Broadcast();
}

void ADefaultGameState::OnRep_AlarmStarted()
{
	// When clients receive AlarmEndTime, notify local listeners
	OnAlarmStarted.Broadcast(AlarmEndTime);
}

void ADefaultGameState::OnRep_AlarmActive()
{
	// If alarm was cleared on server, notify clients
	if (!bAlarmActive)
	{
		OnAlarmCanceled.Broadcast();
	}
	// If bAlarmActive became true the OnRep_AlarmStarted will have already fired for AlarmEndTime.
}

void ADefaultGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADefaultGameState, AlarmEndTime);
	DOREPLIFETIME(ADefaultGameState, bAlarmActive);
	DOREPLIFETIME(ADefaultGameState, AlarmInstigator);
}