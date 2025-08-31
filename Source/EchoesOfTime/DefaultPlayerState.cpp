#include "DefaultPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/DefaultAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/DefaultAttributeSet.h"
#include "Abilities/GameplayAbility.h"
#include "AttributeSet.h"
#include "TimerManager.h"
#include "GameplayTagContainer.h"

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
	if (!HasAuthority()) return;

	for (TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, 0));
	}
}

void ADefaultPlayerState::CopyProperties(APlayerState* NewPlayerState)
{
	Super::CopyProperties(NewPlayerState);

	if (ADefaultPlayerState* PS = Cast<ADefaultPlayerState>(NewPlayerState))
	{
		PS->TeamName = TeamName;
		PS->UpdateTeamGameplayTag();
	}
}

void ADefaultPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
		UpdateTeamGameplayTag();

	if (GetNetMode() == NM_DedicatedServer)
		return;

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
	DOREPLIFETIME(ADefaultPlayerState, TeamName);
}

void ADefaultPlayerState::ServerSetDisplayName_Implementation(const FString& NewName) { ApplyDisplayName(NewName); }
void ADefaultPlayerState::ServerSetAvatarIndex_Implementation(int32 NewIndex) { ApplyAvatarIndex(NewIndex); }
void ADefaultPlayerState::ServerSetReady_Implementation(bool bNewReady) { ApplyReady(bNewReady); }
void ADefaultPlayerState::ServerToggleReady_Implementation() { ApplyReady(!bIsReady); }
void ADefaultPlayerState::ServerSetTeam_Implementation(const FString& NewTeam) { ApplyTeam(NewTeam); }

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

void ADefaultPlayerState::SetReadyLocal(bool bNewReady)
{
	if (GetNetMode() == NM_Standalone || HasAuthority())
	{
		ApplyReady(bNewReady);
	}
}

void ADefaultPlayerState::SetTeamLocal(const FString& NewTeam)
{
	if (GetNetMode() == NM_Standalone || HasAuthority())
	{
		ApplyTeam(NewTeam);
	}
	else
	{
		ServerSetTeam(NewTeam);
	}
}

void ADefaultPlayerState::ApplyTeam(const FString& NewTeam)
{
	if (TeamName == NewTeam) return;
	TeamName = NewTeam;
	UpdateTeamGameplayTag();
	OnRep_TeamName();
}

void ADefaultPlayerState::OnRep_TeamName()
{
	UpdateTeamGameplayTag(); // <--- Add this line!
	OnTeamChanged.Broadcast(this);
	OnPlayerMetaChanged.Broadcast(this);
}

void ADefaultPlayerState::UpdateTeamGameplayTag()
{
	if (!AbilitySystemComponent) return;
	const FGameplayTag PastTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Team.Past")), false);
	const FGameplayTag FutureTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Team.Future")), false);

	if (PastTag.IsValid())   AbilitySystemComponent->RemoveLooseGameplayTag(PastTag);
	if (FutureTag.IsValid()) AbilitySystemComponent->RemoveLooseGameplayTag(FutureTag);

	if (TeamName == "Past" && PastTag.IsValid())
		AbilitySystemComponent->AddLooseGameplayTag(PastTag);
	else if (TeamName == "Future" && FutureTag.IsValid())
		AbilitySystemComponent->AddLooseGameplayTag(FutureTag);
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