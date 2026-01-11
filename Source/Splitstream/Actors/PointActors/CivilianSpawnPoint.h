#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimelineEra.h"
#include "CivilianSpawnPoint.generated.h"

UCLASS()
class SPLITSTREAM_API ACivilianSpawnPoint : public AActor
{
    GENERATED_BODY()

public:
    ACivilianSpawnPoint();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;
};