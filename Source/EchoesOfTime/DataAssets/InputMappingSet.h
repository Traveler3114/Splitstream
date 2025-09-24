#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "InputAction.h"
#include "InputMappingSet.generated.h"

USTRUCT(BlueprintType)
struct FInputActionTagMapping
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UInputAction* InputAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTag InputTag;
};

UCLASS(BlueprintType)
class ECHOESOFTIME_API UInputMappingSet : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FInputActionTagMapping> Mappings;
};