#include "RefPointActor.h"
#include "Kismet/GameplayStatics.h"

FVector ARefPointActor::GetOffsetBetweenFirstTwoRefPoints(UObject* WorldContextObject)
{
    static TMap<UWorld*, FVector> CachedOffsets;

    UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
    if (!World) return FVector::ZeroVector;

    if (const FVector* Cached = CachedOffsets.Find(World))
    {
        return *Cached;
    }

    TArray<AActor*> RefPoints;
    UGameplayStatics::GetAllActorsOfClass(WorldContextObject, ARefPointActor::StaticClass(), RefPoints);
    if (RefPoints.Num() < 2)
    {
        CachedOffsets.Add(World, FVector::ZeroVector);
        return FVector::ZeroVector;
    }

    FVector Offset = (RefPoints[1]->GetActorLocation() - RefPoints[0]->GetActorLocation()).GetAbs() * (-1.0f);
    CachedOffsets.Add(World, Offset);
    return Offset;
}
