#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "NavUtilityLibrary.generated.h"

UCLASS()
class ECHOESOFTIME_API UNavUtilityLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Returns ETA (seconds) for StartActor to reach EndActor at current speed, or -1 if cannot path. */
    UFUNCTION(BlueprintPure, Category="AI|Navigation", meta=(WorldContext="WorldContextObject"))
    static float EstimateTravelTimeBetweenActors(const UObject* WorldContextObject, AActor* StartActor, AActor* EndActor);
};