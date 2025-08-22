#include "PastItemPickup.h"
#include "FutureItemPickup.h"
#include "Engine/World.h"

APastItemPickup::APastItemPickup() {}

void APastItemPickup::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority())
    {
        SpawnLinkedFutureItem();
    }
}

void APastItemPickup::SpawnLinkedFutureItem()
{
    FVector FutureLocation = GetActorLocation() + FVector(200, 0, 0); // Your offset here
    FRotator Rot = GetActorRotation();

    AFutureItemPickup* Future = GetWorld()->SpawnActor<AFutureItemPickup>(AFutureItemPickup::StaticClass(), FutureLocation, Rot);
    if (Future)
    {
        SpawnedFutureItem = Future;
        Future->LinkedPastItem = this;
        // Optionally copy item data
        Future->InitFromItemData(ItemData);
    }
}

// If using Interact or OnPickedUp, destroy future when past is picked up:
void APastItemPickup::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);
    if (SpawnedFutureItem)
    {
        SpawnedFutureItem->OnPastItemPickedUp();
    }
    SpawnedFutureItem = nullptr;
}