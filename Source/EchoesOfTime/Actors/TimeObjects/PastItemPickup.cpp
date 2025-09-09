#include "PastItemPickup.h"
#include "FutureItemPickup.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/PointActors/RefPointActor.h"

APastItemPickup::APastItemPickup() 
{
}

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

    FVector DesiredLocation = GetActorLocation() + FutureItemPickupOffset;

    // ... your line trace logic here if you use it ...
    FVector FutureLocation = DesiredLocation; // or result of your trace

    FRotator ActorRotation = GetActorRotation();
    FRotator MeshRelativeRotation = OverrideMeshComp ? OverrideMeshComp->GetRelativeRotation() : FRotator::ZeroRotator;
    FRotator CombinedRotation = (ActorRotation.Quaternion() * MeshRelativeRotation.Quaternion()).Rotator();


    FTransform FutureTransform = FTransform(CombinedRotation, FutureLocation);

    AFutureItemPickup* Future = GetWorld()->SpawnActorDeferred<AFutureItemPickup>(AFutureItemPickup::StaticClass(), FutureTransform);
    if (Future)
    {
        Future->ItemData = ItemData;
        Future->ItemInstanceID = ItemInstanceID;
        UGameplayStatics::FinishSpawningActor(Future, FutureTransform);
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