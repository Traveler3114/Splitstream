#pragma once

#include "UObject/Interface.h"
#include "IPuzzleCompletionReceiver.generated.h"

/**
 * Interface for actors that receive puzzle completion/reset events.
 *
 * Implement on any actor that should react when an associated puzzle is
 * completed or reset (e.g., doors that open on completion, spawners that
 * pause/resume).
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UPuzzleCompletionReceiver : public UInterface
{
    GENERATED_BODY()
};

class SPLITSTREAM_API IPuzzleCompletionReceiver
{
    GENERATED_BODY()

public:
    /** Called when the associated puzzle is successfully completed. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Puzzle")
    void OnPuzzleCompleted();

    /** Called when the associated puzzle is reset back to its unsolved state. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Puzzle")
    void OnPuzzleReset();
};