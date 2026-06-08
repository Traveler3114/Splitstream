#include "GE_Timer.h"
#include "AbilitySystem/SplitstreamGameplayTags.h"

UGE_Timer::UGE_Timer()
{
    DurationPolicy = EGameplayEffectDurationType::HasDuration;

    FSetByCallerFloat SetByCaller;
    SetByCaller.DataTag = TAG_SetByCaller_Duration;
    DurationMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
}
