#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimelineEra.h"
#include "RandomPointActor.generated.h"

UCLASS()
class SPLITSTREAM_API ARandomPointActor : public AActor
{
    GENERATED_BODY()

public:
    ARandomPointActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnPoint")
    TObjectPtr<class UArrowComponent> ArrowComp; // Add this property
};