#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "PlayerAttributeSet.generated.h"


#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class ECHOESOFTIME_API UPlayerAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:
    UPlayerAttributeSet();

    // Health
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Health)
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, Health)

        UFUNCTION()
    void OnRep_Health(const FGameplayAttributeData& OldValue);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_WalkSpeed)
    FGameplayAttributeData WalkSpeed;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, WalkSpeed)
    UFUNCTION()
    void OnRep_WalkSpeed(const FGameplayAttributeData& OldValue);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_RunSpeed)
    FGameplayAttributeData RunSpeed;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, RunSpeed)
    UFUNCTION()
    void OnRep_RunSpeed(const FGameplayAttributeData& OldValue);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_CrouchSpeed)
    FGameplayAttributeData CrouchSpeed;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, CrouchSpeed)
    UFUNCTION()
    void OnRep_CrouchSpeed(const FGameplayAttributeData& OldValue);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};