#include "ItemBase.h"
#include "InventorySystem/Items/ItemPickup.h"

void UItemBase::OnEquipped_Implementation(AActor* Instigator) {}
void UItemBase::OnUsed_Implementation(AActor* Instigator) {}
void UItemBase::OnDropped_Implementation(AActor* Instigator)
{
    if (!Instigator) return;

    UWorld* World = Instigator->GetWorld();
    if (!World) return;

    // Drop the item a short distance in front of the instigator
    FVector SpawnLocation = Instigator->GetActorLocation() + Instigator->GetActorForwardVector() * 100.0f;
    FRotator SpawnRotation = FRotator::ZeroRotator;

    // Spawn the pickup actor
    AItemPickup* Pickup = World->SpawnActor<AItemPickup>(AItemPickup::StaticClass(), SpawnLocation, SpawnRotation);
    if (Pickup)
    {
        Pickup->InitFromItemData(this);
    }
}