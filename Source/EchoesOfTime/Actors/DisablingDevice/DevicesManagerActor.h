#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DisablingDeviceActor.h"
#include "TimelineEra.h"
#include "DevicesManagerActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPuzzleCompleted);

UCLASS()
class ECHOESOFTIME_API ADevicesManagerActor : public AActor
{
    GENERATED_BODY()

public:
    ADevicesManagerActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Replicated, Category = "Devices")
    TArray<ADisablingDeviceActor*> Devices;

    UPROPERTY(ReplicatedUsing=OnRep_PuzzleCompleted)
    bool bPuzzleCompleted = false;

    UPROPERTY(BlueprintAssignable, Category="Manager")
    FOnPuzzleCompleted OnPuzzleCompleted;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "WirePuzzle|Completion")
    AActor* CompletionTarget = nullptr;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnDeviceStateChanged(ADisablingDeviceActor* ChangedDevice);

    UFUNCTION()
    void OnRep_PuzzleCompleted();

    void CheckPuzzleState();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};