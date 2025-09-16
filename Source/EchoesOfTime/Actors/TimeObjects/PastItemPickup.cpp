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
    FRotator MeshRelativeRotation = OverrideMeshComp ? OverrideMeshComp->GetRelativeRotation() : FRotator::ZeroRotator;
    FRotator CombinedRotation = (ActorRotation.Quaternion() * MeshRelativeRotation.Quaternion()).Rotator();

    FTransform FutureTransform = FTransform(CombinedRotation, FutureLocation);

    AFutureItemPickup* Future = GetWorld()->SpawnActorDeferred<AFutureItemPickup>(AFutureItemPickup::StaticClass(), FutureTransform);
    if (Future)
    {
        Future->ItemData = ItemData;
        Future->ItemInstanceID = ItemInstanceID;

        // --- Copy mesh, scale, and rotation from Past to Future if set ---
        if (OverrideMeshComp && OverrideMeshComp->GetStaticMesh())
        {
            Future->OverrideMeshComp->SetStaticMesh(OverrideMeshComp->GetStaticMesh());
            Future->OverrideMeshComp->SetWorldScale3D(OverrideMeshComp->GetComponentScale());
            Future->OverrideMeshComp->SetRelativeRotation(OverrideMeshComp->GetRelativeRotation());
            // You may also want to copy material overrides:
            for (int32 i = 0; i < OverrideMeshComp->GetNumMaterials(); ++i)
            {
                UMaterialInterface* Mat = OverrideMeshComp->GetMaterial(i);
                if (Mat)
                {
                    Future->OverrideMeshComp->SetMaterial(i, Mat);
                }
            }
        }

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