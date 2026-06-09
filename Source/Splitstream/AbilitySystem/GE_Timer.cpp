#include "GE_Timer.h"
#include "AbilitySystem/SplitstreamGameplayTags.h"
#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"

UGE_Timer::UGE_Timer()
{
    DurationPolicy = EGameplayEffectDurationType::HasDuration;

    FSetByCallerFloat SetByCaller;
    SetByCaller.DataTag = TAG_Effect_Timer;
    DurationMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);

    UAssetTagsGameplayEffectComponent* TagComponent = CreateDefaultSubobject<UAssetTagsGameplayEffectComponent>(TEXT("AssetTags"));
    FInheritedTagContainer TagContainer;
    TagContainer.AddTag(TAG_Effect_Timer);
    TagComponent->SetAndApplyAssetTagChanges(TagContainer);
    GEComponents.Add(TagComponent);
}
