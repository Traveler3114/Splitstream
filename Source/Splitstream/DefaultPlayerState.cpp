// DefaultPlayerState.cpp

#include "DefaultPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/DefaultAbilitySystemComponent.h"
#include "AttributeSet.h"
#include "AbilitySystem/AttributeSets/PlayerAttributeSet.h"
#include "TimerManager.h"

ADefaultPlayerState::ADefaultPlayerState()
{
    AbilitySystemComponent = CreateDefaultSubobject<UDefaultAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    PlayerAttributeSet = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("PlayerAttributeSet"));

    TeamTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Past"));

    bReplicates = true;
}

void ADefaultPlayerState::CopyProperties(APlayerState* NewPlayerState)
{
    Super::CopyProperties(NewPlayerState);

    if (ADefaultPlayerState* PS = Cast<ADefaultPlayerState>(NewPlayerState))
    {
        PS->TeamTag = TeamTag;
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
    DOREPLIFETIME(ADefaultPlayerState, TeamTag);
}

void ADefaultPlayerState::ServerSetDisplayName_Implementation(const FString& NewName) { ApplyDisplayName(NewName); }
void ADefaultPlayerState::ServerSetAvatarIndex_Implementation(int32 NewIndex) { ApplyAvatarIndex(NewIndex); }
void ADefaultPlayerState::ServerSetReady_Implementation(bool bNewReady) { ApplyReady(bNewReady); }
void ADefaultPlayerState::ServerToggleReady_Implementation() { ApplyReady(!bIsReady); }
void ADefaultPlayerState::ServerSetTeam_Implementation(const FGameplayTag& NewTag){ ApplyTeam(NewTag); }

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

void ADefaultPlayerState::SetTeamLocal(const FGameplayTag& NewTag)
{
    if (GetNetMode() == NM_Standalone || HasAuthority())
    {
        ApplyTeam(NewTag);
    }
    else
    {
        ServerSetTeam(NewTag);
    }
}

FString ADefaultPlayerState::GetTeamDisplayName(const FGameplayTag& Tag)
{
    static const FGameplayTag PastTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Past"));
    static const FGameplayTag FutureTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Future"));
    static const FGameplayTag SoloTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Solo"));

    if (Tag == PastTag)   return TEXT("Past");
    if (Tag == FutureTag) return TEXT("Future");
    if (Tag == SoloTag)   return TEXT("Solo");

    return TEXT("Unknown");
}

void ADefaultPlayerState::ApplyTeam(const FGameplayTag& NewTag)
{
    if (TeamTag == NewTag) return;
    TeamTag = NewTag;
    UpdateTeamGameplayTag();
    OnRep_TeamTag();
}

void ADefaultPlayerState::OnRep_TeamTag()
{
    UpdateTeamGameplayTag();
    OnTeamChanged.Broadcast(this);
    OnPlayerMetaChanged.Broadcast(this);
}

void ADefaultPlayerState::UpdateTeamGameplayTag()
{
    if (!AbilitySystemComponent) return;

    // Remove all known team tags (handles future expansion too!)
    TArray<FGameplayTag> AllTeamTags = {
        FGameplayTag::RequestGameplayTag(TEXT("Team.Past")),
        FGameplayTag::RequestGameplayTag(TEXT("Team.Future")),
        FGameplayTag::RequestGameplayTag(TEXT("Team.Solo"))
    };

    for (const FGameplayTag& Tag : AllTeamTags)
        if (Tag.IsValid())
            AbilitySystemComponent->RemoveLooseGameplayTag(Tag);

    // Apply the current team tag if valid
    if (TeamTag.IsValid())
        AbilitySystemComponent->AddLooseGameplayTag(TeamTag);
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