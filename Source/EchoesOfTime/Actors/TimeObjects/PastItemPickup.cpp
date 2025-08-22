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
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            5.f,
            FColor::Green,
            TEXT("PastItemPickup")
        );
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
        // Copy item data
        Future->ItemData = ItemData;
        UGameplayStatics::FinishSpawningActor(Future, FutureTransform);
    }
}

void APastItemPickup::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);
    if (SpawnedFutureItem)
    {
        SpawnedFutureItem->OnPastItemPickedUp();
    }
    SpawnedFutureItem = nullptr;
}