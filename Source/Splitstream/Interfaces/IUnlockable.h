#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IUnlockable.generated.h"

UINTERFACE(Blueprintable)
class SPLITSTREAM_API UUnlockable : public UInterface
{
    GENERATED_BODY()
};

class SPLITSTREAM_API IUnlockable
{
    GENERATED_BODY()

public:
    // Called to unlock with 
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "")
    void UnlockWithAccess(AActor* Interactor);
};