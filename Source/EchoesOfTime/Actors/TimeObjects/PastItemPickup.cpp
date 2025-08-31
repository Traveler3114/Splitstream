#include "PastItemPickup.h"
#include "FutureItemPickup.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/RefPointActor.h"

APastItemPickup::APastItemPickup() {}

void APastItemPickup::BeginPlay()
{
    Super::BeginPlay();
    FutureItemPickupOffset = ARefPointActor::GetOffsetBetweenFirstTwoRefPoints(GetWorld());
    if (HasAuthority())
    {
        SpawnLinkedFutureItem();
    }

}

void APastItemPickup::SpawnLinkedFutureItem()
{
    if (!ItemData) return;

    FVector FutureLocation = GetActorLocation() + FutureItemPickupOffset;
    FRotator Rot = GetActorRotation();
    FTransform FutureTransform = FTransform(Rot, FutureLocation);

    AFutureItemPickup* Future = GetWorld()->SpawnActorDeferred<AFutureItemPickup>(AFutureItemPickup::StaticClass(), FutureTransform);
    if (Future)
    {
        // Only set data on the future item, no back-ref!
        Future->ItemData = ItemData;
        Future->ItemInstanceID = ItemInstanceID;
        UGameplayStatics::FinishSpawningActor(Future, FutureTransform);

        // Store the reference in PastItemPickup
        SpawnedFutureItem = Future;
    }
}

void APastItemPickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ItemInstanceID.IsValid())
    {
        AFutureItemPickup::OnFutureItemInvalidated.Broadcast(ItemInstanceID);
    }
    Super::EndPlay(EndPlayReason);
}