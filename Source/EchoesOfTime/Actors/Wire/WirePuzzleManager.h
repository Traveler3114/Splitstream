#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WireActor.h"
#include "WireDeviceActor.h"
#include "TimelineEra.h"
#include "ProceduralLevelGenerator.h" // Required for FWireSequenceStep usage
#include "WirePuzzleManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWirePuzzleCompleted);

/*
* WirePuzzleManager manages the wire device puzzle sequence.
* Uses order and correct color arrays for per-step validation, currently synced from FWireSequenceStep array in generator.
*/

UCLASS()
class ECHOESOFTIME_API AWirePuzzleManager : public AActor
{
    GENERATED_BODY()
public:
    AWirePuzzleManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "WirePuzzle", Replicated)
    TArray<AWireDeviceActor*> PuzzleDevices;

    UPROPERTY(Replicated)
    TArray<int32> DeviceOrder;  // Order of indices into PuzzleDevices to activate

    UPROPERTY(Replicated)
    TArray<EWireColor> CorrectWireColors; // The expected wire color for each device in the sequence

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WirePuzzle|Audio")
    TArray<class USoundBase*> WireDeviceSounds;

    UPROPERTY(Replicated)
    int32 ProgressIndex = 0;

    UPROPERTY(ReplicatedUsing = OnRep_PuzzleCompleted)
    bool bCompleted = false;

    UPROPERTY(BlueprintAssignable, Category = "WirePuzzle")
    FOnWirePuzzleCompleted OnWirePuzzleCompleted;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "WirePuzzle|Completion")
    AActor* CompletionTarget = nullptr;

    virtual void BeginPlay() override;
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