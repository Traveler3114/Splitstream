#include "ItemBaseWithAbilities.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

void UItemBaseWithAbilities::OnEquipped(AActor* Instigator)
{
    Super::OnEquipped(Instigator);

    IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(Instigator);
    if (!AbilityInterface || !AbilitySet) return;

    UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent();
    if (!ASC) return;

    GrantedAbilityHandles.Empty();

    for (const FAbilityInputSetEntry& Entry : AbilitySet->Abilities)
    {
        if (!Entry.AbilityClass) continue;
        FGameplayAbilitySpec Spec(Entry.AbilityClass, Entry.AbilityLevel, 0);
        Spec.GetDynamicSpecSourceTags().AddTag(Entry.InputTag);

        FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
        GrantedAbilityHandles.Add(Handle);
    }
}

void UItemBaseWithAbilities::OnUnequipped(AActor* Instigator)
{
    Super::OnUnequipped(Instigator);

    IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(Instigator);
    if (!AbilityInterface) return;

    UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent();
    if (!ASC) return;

    for (const FGameplayAbilitySpecHandle& Handle : GrantedAbilityHandles)
    {
        ASC->ClearAbility(Handle);
    }
    GrantedAbilityHandles.Empty();
}

void UItemBaseWithAbilities::OnDroppedWithTeam(AActor* Instigator, FGuid ItemInstanceID, FGameplayTag TeamTag, FVector DropLocation)
{
    Super::OnDroppedWithTeam(Instigator, ItemInstanceID, TeamTag, DropLocation);

    IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(Instigator);
    if (!AbilityInterface) return;

    UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent();
    if (!ASC) return;

    for (const FGameplayAbilitySpecHandle& Handle : GrantedAbilityHandles)
    {
        ASC->ClearAbility(Handle);
    }
    GrantedAbilityHandles.Empty();
}