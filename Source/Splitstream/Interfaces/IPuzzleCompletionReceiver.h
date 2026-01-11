#pragma once

#include "UObject/Interface.h"
#include "IPuzzleCompletionReceiver.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UPuzzleCompletionReceiver : public UInterface
{
    GENERATED_BODY()
};

class SPLITSTREAM_API IPuzzleCompletionReceiver
{
    GENERATED_BODY()

public:
    // Use BlueprintNativeEvent so you can implement in C++ or Blueprint
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Puzzle")
    void OnPuzzleCompleted();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Puzzle")
    void OnPuzzleReset();
};