#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IPuzzleCompletionReceiver.h"
#include "LaserManager.generated.h"

class ALaserSensor;

UCLASS()
class ECHOESOFTIME_API ALaserManager : public AActor, public IPuzzleCompletionReceiver
{
    GENERATED_BODY()

public:
    ALaserManager();

protected:
    virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
    // List of sensors (assign in editor per-instance or via Blueprint)
    UPROPERTY(EditInstanceOnly, Category = "Laser Group")
    TArray<ALaserSensor*> LaserSensors;

    // If true the group will randomly show 'NumToShow' lasers every RandomInterval seconds (server only)
    UPROPERTY(EditAnywhere, Category = "Laser Group")
    bool bRandomize = false;

    // Number of lasers to show when randomizing (clamped to sensor count)
    UPROPERTY(EditAnywhere, Category = "Laser Group", meta = (ClampMin = "0"))
    int32 NumToShow = 5;

    // Interval for randomization
    UPROPERTY(EditAnywhere, Category = "Laser Group", meta = (ClampMin = "0.0"))
    float RandomizeInterval = 2.0f;

    // Should randomization start automatically on BeginPlay (server)
    UPROPERTY(EditAnywhere, Category = "Laser Group")
    bool bStartRandomOnBeginPlay = true;

    virtual void OnPuzzleCompleted_Implementation() override;

private:
    FTimerHandle TimerHandle_Randomize;

    void RandomizeOnce();
    void SetSensorsActiveByIndices(const TArray<int32>& IndicesToActivate);
    void SetAllSensorsActive(bool bActive);
};