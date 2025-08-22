#include "PastItemPickup.h"
#include "FutureItemPickup.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

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
    FVector FutureLocation = GetActorLocation() + FVector(200, 0, 0);
    FRotator Rot = GetActorRotation();
    FTransform FutureTransform = FTransform(Rot, FutureLocation);

    AFutureItemPickup* Future = GetWorld()->SpawnActorDeferred<AFutureItemPickup>(AFutureItemPickup::StaticClass(), FutureTransform);
    if (Future)
    {
        SpawnedFutureItem = Future;
        Future->LinkedPastItem = this;
        Future->ItemData = ItemData;
        UGameplayStatics::FinishSpawningActor(Future, FutureTransform);
    }
}

void APastItemPickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (SpawnedFutureItem)
    {
        SpawnedFutureItem->OnPastItemPickedUp();
    }
    Super::EndPlay(EndPlayReason);
}