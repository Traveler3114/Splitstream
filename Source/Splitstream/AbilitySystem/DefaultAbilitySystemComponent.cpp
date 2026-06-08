#include "DefaultAbilitySystemComponent.h"
#include "GameplayEffect.h"

void UDefaultAbilitySystemComponent::GrantAbilitiesFromSet(const UAbilityInputSet* Set)
{
    if (!Set) return;
    for (const FAbilityInputSetEntry& Entry : Set->Abilities)
    {
        if (!Entry.AbilityClass) continue;
        FGameplayAbilitySpec Spec(Entry.AbilityClass, Entry.AbilityLevel, 0);
        if (Entry.InputTag.IsValid())
        {
            Spec.GetDynamicSpecSourceTags().AddTag(Entry.InputTag);
        }
        GiveAbility(Spec);
    }
}

void UDefaultAbilitySystemComponent::GrantAbilitiesFromDefaultSet(const UDefaultGASet* Set)
{
    if (!Set) return;
    for (const TSubclassOf<UGameplayAbility>& AbilityClass : Set->GrantedAbilities)
    {
        if (!AbilityClass) continue;
        FGameplayAbilitySpec Spec(AbilityClass, 1, 0);
        GiveAbility(Spec);
    }
}

void UDefaultAbilitySystemComponent::InitializeAttributes(TSubclassOf<UGameplayEffect> InitGE)
{
    if (!InitGE || GetOwnerRole() != ROLE_Authority) return;

    FGameplayEffectContextHandle Ctx = MakeEffectContext();
    FGameplayEffectSpecHandle Spec = MakeOutgoingSpec(InitGE, 1.f, Ctx);
    if (Spec.IsValid())
    {
        ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
    }
}

void UDefaultAbilitySystemComponent::HandleAbilityInputPressed(FGameplayTag InputTag)
{
    for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
    {
        if (Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
        {
            TryActivateAbility(Spec.Handle);
        }
    }
}

void UDefaultAbilitySystemComponent::HandleAbilityInputReleased(FGameplayTag InputTag)
{
    for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
    {
        if (Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag) && Spec.IsActive())
        {
            CancelAbilityHandle(Spec.Handle);
        }
    }
}

void UDefaultAbilitySystemComponent::ServerHandleClientEvent_Implementation(FGameplayTag EventTag, FGameplayEventData EventData)
{
    HandleGameplayEvent(EventTag, &EventData);
}