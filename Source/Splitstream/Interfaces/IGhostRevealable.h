#pragma once

#include "UObject/Interface.h"
#include "IGhostRevealable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UGhostRevealable : public UInterface
{
    GENERATED_BODY()
};

class IGhostRevealable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ghost")
    void SetGhostRevealed(bool bRevealed);
};