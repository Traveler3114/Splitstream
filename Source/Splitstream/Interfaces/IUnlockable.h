#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IUnlockable.generated.h"

/**
 * Interface for actors that can be unlocked by a player with the correct access
 * (e.g., keycard scanners, locked doors).
 */
UINTERFACE(Blueprintable)
class SPLITSTREAM_API UUnlockable : public UInterface
{
    GENERATED_BODY()
};

class SPLITSTREAM_API IUnlockable
{
    GENERATED_BODY()

public:
    /**
     * Called to unlock this actor using the interactor's credentials or held item.
     * @param Interactor  The actor requesting the unlock (usually the player pawn).
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "")
    void UnlockWithAccess(AActor* Interactor);
};