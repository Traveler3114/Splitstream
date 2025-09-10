#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimelineEra.h"
#include "RandomPointActor.generated.h"

UCLASS()
class ECHOESOFTIME_API ARandomPointActor : public AActor
{
    GENERATED_BODY()

public:
    ARandomPointActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;
};