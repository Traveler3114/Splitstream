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
    if (!ItemData) return;

    FVector FutureLocation = GetActorLocation() + FVector(0.0f, -4320.0f, 0.0f);
    FRotator Rot = GetActorRotation();
    FTransform FutureTransform = FTransform(Rot, FutureLocation);

    AFutureItemPickup* Future = GetWorld()->SpawnActorDeferred<AFutureItemPickup>(AFutureItemPickup::StaticClass(), FutureTransform);
    if (Future)
    {
        Future->LinkedPastItem = this;
        Future->ItemData = ItemData;
        UGameplayStatics::FinishSpawningActor(Future, FutureTransform);
    }
}

void APastItemPickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ItemData)
    {
        // Broadcast invalidation to any future item with the same ItemInstanceID
        AFutureItemPickup::OnFutureItemInvalidated.Broadcast(ItemData->ItemInstanceID);
    }
    Super::EndPlay(EndPlayReason);
}
