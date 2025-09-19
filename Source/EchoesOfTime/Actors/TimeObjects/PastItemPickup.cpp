#include "PastItemPickup.h"
#include "FutureItemPickup.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/PointActors/RefPointActor.h"

APastItemPickup::APastItemPickup()
{
    UE_LOG(LogTemp, Warning, TEXT("[APastItemPickup::APastItemPickup] Constructed!"));
}

void APastItemPickup::BeginPlay()
{
    Super::BeginPlay();
    FutureItemPickupOffset = ARefPointActor::GetOffsetBetweenFirstTwoRefPoints(GetWorld());
    UE_LOG(LogTemp, Warning, TEXT("[APastItemPickup::BeginPlay] FutureItemPickupOffset: %s"), *FutureItemPickupOffset.ToString());
    if (HasAuthority())
    {
        SpawnLinkedFutureItem();
    }
}

void APastItemPickup::SpawnLinkedFutureItem()
{
    UE_LOG(LogTemp, Warning, TEXT("[APastItemPickup::SpawnLinkedFutureItem] Called!"));
    if (!ItemData) {
        UE_LOG(LogTemp, Error, TEXT("[APastItemPickup::SpawnLinkedFutureItem] ItemData is nullptr!"));
        return;
    }

    FVector DesiredLocation = GetActorLocation() + FutureItemPickupOffset;
    DesiredLocation.Z = FMath::Max(DesiredLocation.Z, 0.0f);
    FVector FutureLocation = DesiredLocation;
    FRotator ActorRotation = GetActorRotation();
    FTransform FutureTransform = FTransform(ActorRotation, FutureLocation);

    UE_LOG(LogTemp, Warning, TEXT("[APastItemPickup::SpawnLinkedFutureItem] Spawning at location: %s, rotation: %s"),
        *FutureLocation.ToString(), *ActorRotation.ToString());

    AFutureItemPickup* Future = GetWorld()->SpawnActorDeferred<AFutureItemPickup>(AFutureItemPickup::StaticClass(), FutureTransform);
    if (Future)
    {
        Future->ItemData = ItemData;
        Future->ItemInstanceID = ItemInstanceID;

        UGameplayStatics::FinishSpawningActor(Future, FutureTransform);
        SpawnedFutureItem = Future;

        UE_LOG(LogTemp, Warning, TEXT("[APastItemPickup::SpawnLinkedFutureItem] FutureItem spawned: %s"), *Future->GetName());

        if (OverrideMeshComp && OverrideMeshComp->GetStaticMesh() && Future->OverrideMeshComp)
        {
            Future->OverrideMeshComp->SetStaticMesh(OverrideMeshComp->GetStaticMesh());
            Future->OverrideMeshComp->SetWorldScale3D(OverrideMeshComp->GetComponentScale());
            Future->OverrideMeshComp->SetRelativeRotation(OverrideMeshComp->GetRelativeRotation());
            for (int32 i = 0; i < OverrideMeshComp->GetNumMaterials(); ++i)
            {
                UMaterialInterface* Mat = OverrideMeshComp->GetMaterial(i);
                if (Mat)
                {
                    Future->OverrideMeshComp->SetMaterial(i, Mat);
                }
            }
            UE_LOG(LogTemp, Warning, TEXT("[APastItemPickup::SpawnLinkedFutureItem] Mesh and materials copied to FutureItem."));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[APastItemPickup::SpawnLinkedFutureItem] Mesh copy failed (missing mesh or comp)!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[APastItemPickup::SpawnLinkedFutureItem] Failed to spawn AFutureItemPickup!"));
    }
}

void APastItemPickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ItemInstanceID.IsValid())
    {
        AFutureItemPickup::OnFutureItemInvalidated.Broadcast(ItemInstanceID);
        UE_LOG(LogTemp, Warning, TEXT("[APastItemPickup::EndPlay] FutureItemInvalidated broadcast for %s!"), *ItemInstanceID.ToString());
    }
    Super::EndPlay(EndPlayReason);
}

#if WITH_EDITOR
void APastItemPickup::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    ItemInstanceID = FGuid::NewGuid();
    UE_LOG(LogTemp, Warning, TEXT("[APastItemPickup::OnConstruction] New ItemInstanceID: %s"), *ItemInstanceID.ToString());
}
#endif