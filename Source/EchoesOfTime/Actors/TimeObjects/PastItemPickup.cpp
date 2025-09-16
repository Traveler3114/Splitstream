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
    FVector FutureLocation = DesiredLocation;
    FRotator ActorRotation = GetActorRotation();
    FTransform FutureTransform = FTransform(ActorRotation, FutureLocation);

    AFutureItemPickup* Future = GetWorld()->SpawnActorDeferred<AFutureItemPickup>(AFutureItemPickup::StaticClass(), FutureTransform);
    if (Future)
    {
        Future->ItemData = ItemData;
        Future->ItemInstanceID = ItemInstanceID;

        UGameplayStatics::FinishSpawningActor(Future, FutureTransform);
        SpawnedFutureItem = Future;

        // Copy mesh, scale, and relative rotation from Past to Future if set and valid
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
        }
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

#if WITH_EDITOR
void APastItemPickup::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    ItemInstanceID = FGuid::NewGuid();
}
#endif