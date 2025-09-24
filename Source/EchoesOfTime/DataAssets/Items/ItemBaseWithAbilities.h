#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "DataAssets/AbilitySets/AbilityInputSet.h"
#include "GameplayAbilitySpec.h"
#include "ItemBaseWithAbilities.generated.h"

UCLASS(BlueprintType)
class ECHOESOFTIME_API UItemBaseWithAbilities : public UItemBase
{
    GENERATED_BODY()
public:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
    UAbilityInputSet* AbilitySet;

    UPROPERTY(Transient)
    TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;

    virtual void OnEquipped(AActor* Instigator) override;
    virtual void OnUnequipped(AActor* Instigator) override;
    virtual void OnDropped(AActor* Instigator, FGuid ItemInstanceID, FVector DropLocation) override;
    virtual void OnDroppedWithTeam(AActor* Instigator, FGuid ItemInstanceID, FGameplayTag TeamTag, FVector DropLocation) override;

protected:
    void RemoveGrantedAbilities(AActor* Instigator);
};