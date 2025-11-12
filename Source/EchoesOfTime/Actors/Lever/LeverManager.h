#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LeverActor.h" // Needed for delegate type!
#include "LeverManager.generated.h"

class ALeverActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeverPuzzleCompleted); // This is fine to repeat (different delegate name)

UCLASS()
class ECHOESOFTIME_API ALeverManager : public AActor
{
    GENERATED_BODY()

public:
    ALeverManager();

    // Manually select levers in editor
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Replicated, Category = "Lever")
    TArray<ALeverActor*> PuzzleLevers;
    
    UPROPERTY(Replicated)
    TArray<int32> Order; // Provided by procedural generator

    UPROPERTY(Replicated)
    int32 ProgressIndex = 0;

    UPROPERTY(ReplicatedUsing=OnRep_PuzzleCompleted)
    bool bCompleted = false;

    UPROPERTY(BlueprintAssignable, Category="LeverManager")
    FOnLeverPuzzleCompleted OnLeverPuzzleCompleted;

    void SetupPuzzle(const TArray<int32>& OrderSeq);

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Lever|Completion")
    AActor* CompletionTarget = nullptr;

protected:
    UFUNCTION()
    void OnRep_PuzzleCompleted();

    UFUNCTION()
    void OnLeverInteracted(ALeverActor* Lever); // Matches delegate!

    void ActivateLever(ALeverActor* Lever);
    void ResetPuzzle();
    void CompletePuzzle();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};