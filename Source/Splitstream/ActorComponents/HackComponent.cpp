#include "HackComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/SplitstreamGameplayTags.h"

UHackComponent::UHackComponent()
{
    SetIsReplicatedByDefault(true);
    bHacked = false;
    HackDuration = 10.f;
}

void UHackComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UHackComponent::SetHacked()
{
    if (GetOwnerRole() != ROLE_Authority) return;
    if (bHacked) return;
    bHacked = true;
    OnHackComplete.Broadcast();
}

void UHackComponent::OnRep_Hacked()
{
    if (bHacked)
    {
        OnHackComplete.Broadcast();
    }
}

void UHackComponent::Interact(AActor* Interactor)
{
    if (bHacked) return;

    if (IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(Interactor))
    {
        if (UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent())
        {
            FGameplayEventData EventData;
            EventData.Instigator = Interactor;
            EventData.OptionalObject = GetOwner();

            ASC->HandleGameplayEvent(
                TAG_Character_Ability_Hack,
                &EventData
            );
        }
    }
}

void UHackComponent::CancelInteract(AActor* Interactor)
{
    if (Interactor)
    {
        if (IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(Interactor))
        {
            if (UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent())
            {
                FGameplayTagContainer CancelTags;
                CancelTags.AddTag(TAG_Character_Ability_Hack);
                ASC->CancelAbilities(&CancelTags);
            }
        }
    }
}

void UHackComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UHackComponent, bHacked);
}