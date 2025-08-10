#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/DefaultAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/DefaultAttributeSet.h"
#include "DefaultPlayerState.generated.h"

/**
 * PlayerState that owns the GAS system
 */
UCLASS()
class ECHOESOFTIME_API ADefaultPlayerState : public APlayerState, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    ADefaultPlayerState();

    // IAbilitySystemInterface
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    TObjectPtr<UDefaultAbilitySystemComponent> AbilitySystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    TObjectPtr<UDefaultAttributeSet> AttributeSet;
};