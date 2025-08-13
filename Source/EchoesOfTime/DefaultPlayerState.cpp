#include "DefaultPlayerState.h"
#include "Net/UnrealNetwork.h"

ADefaultPlayerState::ADefaultPlayerState() {}

void ADefaultPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADefaultPlayerState, DisplayName);
	DOREPLIFETIME(ADefaultPlayerState, AvatarIndex);
	DOREPLIFETIME(ADefaultPlayerState, bIsReady);
}

void ADefaultPlayerState::ServerSetDisplayName_Implementation(const FString& NewName) { ApplyDisplayName(NewName); }
void ADefaultPlayerState::ServerSetAvatarIndex_Implementation(int32 NewIndex) { ApplyAvatarIndex(NewIndex); }
void ADefaultPlayerState::ServerSetReady_Implementation(bool bNewReady) { ApplyReady(bNewReady); }
void ADefaultPlayerState::ServerToggleReady_Implementation() { ApplyReady(!bIsReady); }

void ADefaultPlayerState::ApplyDisplayName(const FString& NewName)
{
	if (DisplayName == NewName) return;
	DisplayName = NewName;
	OnRep_Meta();
}

void ADefaultPlayerState::ApplyAvatarIndex(int32 NewIndex)
{
	if (AvatarIndex == NewIndex) return;
	AvatarIndex = NewIndex;
	OnRep_Meta();
}

void ADefaultPlayerState::ApplyReady(bool bNewReady)
{
	if (bIsReady == bNewReady) return;
	bIsReady = bNewReady;
	OnRep_Ready();
}

void ADefaultPlayerState::OnRep_Meta()
{
	OnPlayerMetaChanged.Broadcast(this);
}

void ADefaultPlayerState::OnRep_Ready()
{
	OnReadyChanged.Broadcast(this);
}