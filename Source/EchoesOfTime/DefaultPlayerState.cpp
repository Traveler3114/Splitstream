#include "DefaultPlayerState.h"

ADefaultPlayerState::ADefaultPlayerState()
{
    AbilitySystemComponent = CreateDefaultSubobject<UDefaultAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AttributeSet = CreateDefaultSubobject<UDefaultAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* ADefaultPlayerState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}