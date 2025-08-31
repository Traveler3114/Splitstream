#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RefPointActor.generated.h"

UCLASS()
class ECHOESOFTIME_API ARefPointActor : public AActor
{
    GENERATED_BODY()
public:
    // Static function to map a location from one reference to another
// RefPointActor.h
    UFUNCTION(BlueprintCallable, Category = "MapLocation")
    static FVector GetOffsetBetweenFirstTwoRefPoints(UObject* WorldContextObject);
};