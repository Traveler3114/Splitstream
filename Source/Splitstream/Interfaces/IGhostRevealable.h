#pragma once

#include "UObject/Interface.h"
#include "IGhostRevealable.generated.h"

/**
 * Interface for actors that can be revealed or hidden as "ghosts" in the
 * other timeline era. Used by the ghost/mirror system to toggle visibility
 * of mirrored representations.
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UGhostRevealable : public UInterface
{
    GENERATED_BODY()
};

class IGhostRevealable
{
    GENERATED_BODY()

public:
    /**
     * Sets whether this actor's ghost representation should be visible.
     * @param bRevealed  True to reveal the ghost, false to hide it.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ghost")
    void SetGhostRevealed(bool bRevealed);
};