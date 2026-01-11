#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WireActor.h"
#include "WireDeviceActor.h"
#include "TimelineEra.h"
#include "Actors/ProceduralLevelGenerator.h"
#include "WirePuzzleManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWirePuzzleCompleted);

/*
* WirePuzzleManager manages the wire device puzzle sequence.
* Uses order and correct color arrays for per-step validation, currently synced from FWireSequenceStep array in generator.
*/

UCLASS()
class SPLITSTREAM_API AWirePuzzleManager : public AActor
{
    GENERATED_BODY()
public:
    AWirePuzzleManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "WirePuzzle", Replicated)
    TArray<AWireDeviceActor*> PuzzleDevices;

    UPROPERTY(ReplicatedUsing = OnRep_PuzzleCompleted)
    bool bCompleted = false;

    UPROPERTY(BlueprintAssignable, Category = "WirePuzzle")
    FOnWirePuzzleCompleted OnWirePuzzleCompleted;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "WirePuzzle|Completion")
    AActor* CompletionTarget = nullptr;

    virtual void BeginPlay() override;
    void SetupPuzzle();

    UPROPERTY(Replicated)
    TArray<EWireColor> RequiredColors;

    // Colors that have already been correctly cut
    UPROPERTY(Replicated)
    TArray<EWireColor> CompletedColors;

    UPROPERTY(Replicated)
    int32 ProgressIndex = 0;

    //void HighlightNextCorrectWire();

protected:
    UFUNCTION()
    void OnRep_PuzzleCompleted();

    UFUNCTION()
    void OnWireCut(AWireActor* CutWire);

    void ResetPuzzle();
    void CompletePuzzle();

    int32 GetDeviceIndexForWire(AWireActor* Wire) const;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};