#include "LaserManager.h"
#include "LaserSensor.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h" // FMath

ALaserManager::ALaserManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
}

void ALaserManager::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        NumToShow = FMath::Clamp(NumToShow, 0, LaserSensors.Num());

        if (bRandomize && bStartRandomOnBeginPlay && RandomizeInterval > 0.f)
        {
            GetWorldTimerManager().SetTimer(TimerHandle_Randomize, this, &ALaserManager::RandomizeOnce, RandomizeInterval, true);
            RandomizeOnce();
        }
        else
        {
            if (LaserSensors.Num() > 0)
            {
                if (NumToShow >= LaserSensors.Num())
                {
                    SetAllSensorsActive(true);
                }
                else if (NumToShow <= 0)
                {
                    SetAllSensorsActive(false);
                }
                else
                {
                    TArray<int32> Indices;
                    for (int32 i = 0; i < LaserSensors.Num(); ++i)
                    {
                        if (i < NumToShow) Indices.Add(i);
                    }
                    SetSensorsActiveByIndices(Indices);
                }
            }
        }
    }
}

void ALaserManager::OnConstruction(const FTransform& Transform)
{
    //LaserSensors.Empty();

    //TArray<UChildActorComponent*> ChildComps;
    //GetComponents<UChildActorComponent>(ChildComps);

    //for (UChildActorComponent* CAC : ChildComps)
    //{
    //    if (!CAC) continue;
    //    AActor* Child = CAC->GetChildActor();
    //    if (!Child)
    //    {
    //        // If called in OnConstruction, child actor may be null until after spawning; it's safe to skip
    //        continue;
    //    }

    //    if (ALaserSensor* Sensor = Cast<ALaserSensor>(Child))
    //    {
    //        LaserSensors.Add(Sensor);
    //    }
    //}
}

void ALaserManager::RandomizeOnce()
{
    if (!HasAuthority() || LaserSensors.Num() == 0)
    {
        return;
    }

    int32 Count = LaserSensors.Num();
    int32 K = FMath::Clamp(NumToShow, 0, Count);

    // Build indices [0..Count-1]
    TArray<int32> Indices;
    Indices.Reserve(Count);
    for (int32 i = 0; i < Count; ++i) Indices.Add(i);

    // Fisher-Yates shuffle (no external headers required)
    for (int32 i = Count - 1; i > 0; --i)
    {
        int32 j = FMath::RandRange(0, i);
        Indices.Swap(i, j);
    }

    // Pick first K indices to activate
    TArray<int32> ToActivate;
    ToActivate.Append(Indices.GetData(), K);

    SetSensorsActiveByIndices(ToActivate);
}

void ALaserManager::SetSensorsActiveByIndices(const TArray<int32>& IndicesToActivate)
{
    // Build a set for quick lookup
    TSet<int32> ActivateSet(IndicesToActivate);

    for (int32 i = 0; i < LaserSensors.Num(); ++i)
    {
        ALaserSensor* Sensor = LaserSensors[i];
        if (Sensor)
        {
            const bool bShouldBeActive = ActivateSet.Contains(i);
            // Use SetActive (it will forward to server RPC if necessary). We are server so this will set directly.
            Sensor->SetActive(bShouldBeActive);
        }
    }
}

void ALaserManager::SetAllSensorsActive(bool bActive)
{
    for (ALaserSensor* Sensor : LaserSensors)
    {
        if (Sensor)
        {
            Sensor->SetActive(bActive);
        }
    }
}