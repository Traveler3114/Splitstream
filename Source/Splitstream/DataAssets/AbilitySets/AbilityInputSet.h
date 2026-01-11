#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AbilityInputSet.generated.h"

USTRUCT(BlueprintType)
struct FAbilityInputSetEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
    TSubclassOf<class UGameplayAbility> AbilityClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
    int32 AbilityLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
    FGameplayTag InputTag;
};

UCLASS(BlueprintType)
class ECHOESOFTIME_API UAbilityInputSet : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    TArray<FAbilityInputSetEntry> Abilities;
};