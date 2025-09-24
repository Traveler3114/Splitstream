#include "ItemBase.h"
#include "GameplayTagContainer.h"
#include "Actors/ItemPickup.h"
#include "Actors/TimeObjects/PastItemPickup.h"
#include "Actors/TimeObjects/FutureItemPickup.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

void UItemBase::OnEquipped(AActor* Instigator) {}
void UItemBase::OnUnequipped(AActor* Instigator) {}

void UItemBase::OnUsed(AActor* Instigator) {}

void UItemBase::OnDropped(AActor* Instigator, FGuid ItemInstanceID, FVector DropLocation)
{
    if (!Instigator) return;
    UWorld* World = Instigator->GetWorld();
    if (!World) return;

    FRotator SpawnRotation = PickupMeshRotation;
    FTransform SpawnTransform = FTransform(SpawnRotation, DropLocation);

    AItemPickup* Pickup = World->SpawnActorDeferred<AItemPickup>(AItemPickup::StaticClass(), SpawnTransform);
    if (Pickup)
    {
        Pickup->ItemData = this;
        Pickup->ItemInstanceID = ItemInstanceID;
        UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);
    }
}

void UItemBase::OnDroppedWithTeam(AActor* Instigator, FGuid ItemInstanceID, FGameplayTag TeamTag, FVector DropLocation)
{
    if (!Instigator) return;
    UWorld* World = Instigator->GetWorld();
    if (!World) return;

    FRotator SpawnRotation = PickupMeshRotation;
	DropLocation.Z= FMath::Max(DropLocation.Z, 0.0f); // Slightly raise the drop location to avoid clipping into the ground
    FTransform SpawnTransform = FTransform(SpawnRotation, DropLocation);
    static FGameplayTag PastTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Past"));
    static FGameplayTag FutureTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Future"));

    if (TeamTag == PastTag)
    {
        APastItemPickup* Pickup = World->SpawnActorDeferred<APastItemPickup>(APastItemPickup::StaticClass(), SpawnTransform);
        if (Pickup)
        {
            Pickup->ItemData = this;
            Pickup->ItemInstanceID = ItemInstanceID;
            UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);
        }
    }
    else if (TeamTag == FutureTag)
    {
        AFutureItemPickup* Pickup = World->SpawnActorDeferred<AFutureItemPickup>(AFutureItemPickup::StaticClass(), SpawnTransform);
        if (Pickup)
        {
            Pickup->ItemData = this;
            Pickup->ItemInstanceID = ItemInstanceID;
            UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);
        }
    }
    else
    {
        AItemPickup* Pickup = World->SpawnActorDeferred<AItemPickup>(AItemPickup::StaticClass(), SpawnTransform);
        if (Pickup)
        {
            Pickup->ItemData = this;
            Pickup->ItemInstanceID = ItemInstanceID;
            UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);
        }
    }
}