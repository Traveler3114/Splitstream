#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WireActor.h"
#include "WireDeviceActor.h"
#include "TimelineEra.h"
#include "WirePuzzleManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWirePuzzleCompleted);

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
    TArray<int32> DeviceOrder;

    UPROPERTY(Replicated)
    TArray<EWireColor> CorrectWireColors;

    UPROPERTY(Replicated)
    int32 ProgressIndex = 0;

    UPROPERTY(ReplicatedUsing = OnRep_PuzzleCompleted)
    bool bCompleted = false;

    UPROPERTY(BlueprintAssignable, Category = "WirePuzzle")
    FOnWirePuzzleCompleted OnWirePuzzleCompleted;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "WirePuzzle|Completion")
    AActor* CompletionTarget = nullptr;

	virtual void BeginPlay() override;
protected:
    UFUNCTION()
    void OnRep_PuzzleCompleted();

    UFUNCTION()
    void OnWireCut(AWireActor* CutWire);

    void ResetPuzzle();
    void CompletePuzzle();
    //void HighlightNextCorrectWire();

    int32 GetDeviceIndexForWire(AWireActor* Wire) const;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};