#include "RefPointActor.h"
#include "Kismet/GameplayStatics.h"

FVector ARefPointActor::GetOffsetBetweenFirstTwoRefPoints(UObject* WorldContextObject)
{
    TArray<AActor*> RefPoints;
    UGameplayStatics::GetAllActorsOfClass(WorldContextObject, ARefPointActor::StaticClass(), RefPoints);
    if (RefPoints.Num() < 2)
    {
        return FVector::ZeroVector;
    }
    return RefPoints[1]->GetActorLocation() - RefPoints[0]->GetActorLocation();
}