#include "SearchableItemSpawnPoint.h"

ASearchableItemSpawnPoint::ASearchableItemSpawnPoint()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ASearchableItemSpawnPoint::BeginPlay()
{
    Super::BeginPlay();
    // Nothing needed for now, but could be extended
}