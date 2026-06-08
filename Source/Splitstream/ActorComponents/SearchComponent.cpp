#include "SearchComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/SplitstreamGameplayTags.h"

USearchComponent::USearchComponent()
{
    SetIsReplicatedByDefault(true);
    bSearched = false;
    SearchDuration = 10.f;
}

void USearchComponent::BeginPlay()
{
    Super::BeginPlay();
}

void USearchComponent::SetSearched()
{
    if (GetOwner() && !GetOwner()->HasAuthority()) return;

    if (bAllowMultipleSearches)
    {
        bSearched = !bSearched;
    }
    else
    {
        if (bSearched) return;
        bSearched = true;
    }
    OnSearchComplete.Broadcast();
}

void USearchComponent::OnRep_Searched()
{
    OnSearchComplete.Broadcast();
}

void USearchComponent::Interact(AActor* Interactor)
{
    if (bSearched && !bAllowMultipleSearches)
    {
        return;
    }

    if (GetOwner() && GetOwner()->HasAuthority())
    {
        LastInteractor = Interactor;
    }

    if (IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(Interactor))
    {
        if (UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent())
        {
            FGameplayEventData EventData;
            EventData.Instigator = Interactor;
            EventData.OptionalObject = GetOwner();
            ASC->HandleGameplayEvent(
                TAG_Character_Ability_Search,
                &EventData
            );
        }
    }
}

void USearchComponent::CancelInteract(AActor* Interactor)
{
    if (Interactor)
    {
        if (IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(Interactor))
        {
            if (UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent())
            {
                FGameplayTagContainer CancelTags;
                CancelTags.AddTag(TAG_Character_Ability_Search);
                ASC->CancelAbilities(&CancelTags);
            }
        }
    }
}

void USearchComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(USearchComponent, bSearched);
}