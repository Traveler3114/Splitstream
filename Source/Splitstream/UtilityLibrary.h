#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UtilityLibrary.generated.h"

UCLASS()
class SPLITSTREAM_API UUtilityLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Returns ETA (seconds) for StartActor to reach EndActor at current speed, or -1 if cannot path. */
    UFUNCTION(BlueprintPure, Category="AI|igation", meta=(WorldContext="WorldContextObject"))
    static float EstimateTravelTimeBetweenActors(const UObject* WorldContextObject, AActor* StartActor, AActor* EndActor);

    UFUNCTION(BlueprintCallable, Category = "Repairable")
    static void RegisterRepairable(UObject* WorldContext, AActor* Repairable);

    UFUNCTION(BlueprintCallable, Category = "Repairable")
    static void UnregisterRepairable(UObject* WorldContext, AActor* Repairable);

    UFUNCTION(BlueprintCallable, Category = "Detector", meta = (WorldContext = "WorldContextObject"))
    static void RegisterDetector(UObject* WorldContextObject, AActor* Detector);

    UFUNCTION(BlueprintCallable, Category = "Detector", meta = (WorldContext = "WorldContextObject"))
    static void UnregisterDetector(UObject* WorldContextObject, AActor* Detector);
};