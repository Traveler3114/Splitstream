#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DataAssets/ItemBase.h"
#include "IRequiresItem.generated.h"

// Unreal interface class
UINTERFACE(MinimalAPI, Blueprintable)
class URequiresItem : public UInterface
{
    GENERATED_BODY()
};

// C++ pure interface
class IRequiresItem
{
    GENERATED_BODY()

public:
    // Return true if the given item is correct for this actor
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item Requirement")
    bool IsCorrectItem(UItemBase* Item) const;
};