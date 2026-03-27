#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ISettingsTabInterface.generated.h"

/**
 * Interface for actors that can be unlocked by a player with the correct access
 * (e.g., keycard scanners, locked doors).
 */
UINTERFACE(Blueprintable)
class SPLITSTREAM_API USettingsTabInterface : public UInterface
{
    GENERATED_BODY()
};

class SPLITSTREAM_API ISettingsTabInterface
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Settings Tab")
    void ApplySettings();
};