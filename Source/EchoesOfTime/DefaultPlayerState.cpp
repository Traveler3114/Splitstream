#include "DefaultPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/DefaultAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/DefaultAttributeSet.h"
#include "Abilities/GameplayAbility.h"
#include "AttributeSet.h"
#include "TimerManager.h"

ADefaultPlayerState::ADefaultPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UDefaultAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UDefaultAttributeSet>(TEXT("AttributeSet"));

	bReplicates = true;
}

void ADefaultPlayerState::GiveAbilities()
{
	if (!HasAuthority()) return; // Only run on server

	for (TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, 0));
	}
}

void ADefaultPlayerState::BeginPlay()
{
	Super::BeginPlay();

	// Only do avatar fetch on clients (no need on dedicated server)
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// Schedule a call next tick to request the avatar from BP, to ensure UniqueId is valid
	FTimerHandle Tmp;
	GetWorldTimerManager().SetTimerForNextTick([this]()
		{
			BP_RequestAvatar();
		});
}

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

void ADefaultPlayerState::SetAvatarTexture(UTexture2D* InTexture)
{
	if (AvatarTexture == InTexture) return;
	AvatarTexture = InTexture;
	OnAvatarChanged.Broadcast(this);
	OnPlayerMetaChanged.Broadcast(this);
}

void ADefaultPlayerState::SetReadyLocal(bool bNewReady)
{
	// In Standalone there is no RPC; also allow direct authority updates
	if (GetNetMode() == NM_Standalone || HasAuthority())
	{
		ApplyReady(bNewReady);
	}
	// else: in networked clients, use ServerSetReady instead (call site handles this)
}