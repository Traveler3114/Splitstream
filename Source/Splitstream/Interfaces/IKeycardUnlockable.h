#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IKeycardUnlockable.generated.h"

UINTERFACE(Blueprintable)
class SPLITSTREAM_API UKeycardUnlockable : public UInterface
{
    GENERATED_BODY()
};

class SPLITSTREAM_API IKeycardUnlockable
{
    GENERATED_BODY()

public:
    // Called to unlock with keycard
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Keycard")
    void UnlockWithKeycard(AActor* Interactor);

    // Optionally, expose requires-keycard property
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Keycard")
    bool RequiresKeycard() const;
};