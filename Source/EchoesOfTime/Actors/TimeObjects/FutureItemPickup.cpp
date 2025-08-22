#include "FutureItemPickup.h"
#include "PastItemPickup.h"
#include "InventorySystem/InventoryComponent.h"
#include "DrawDebugHelpers.h"

FFutureItemInvalidated AFutureItemPickup::OnFutureItemInvalidated;

AFutureItemPickup::AFutureItemPickup()
{
}

void AFutureItemPickup::BeginPlay()
{
    Super::BeginPlay();
}

void AFutureItemPickup::Interact_Implementation(AActor* Interactor)
{
    if (!ItemData || !Interactor) return;

    UInventoryComponent* Inventory = Interactor->FindComponentByClass<UInventoryComponent>();
    if (Inventory)
    {
        // Register for invalidation event here!
        Inventory->RegisterFutureInstance(ItemData->ItemInstanceID);

        if (Inventory->AddItem(ItemData->GetClass(), ItemData->ItemInstanceID))
        {
            Destroy();
        }
    }
}

void AFutureItemPickup::OnPastItemPickedUp()
{
    InvalidateFromTimeline();
}

void AFutureItemPickup::InvalidateFromTimeline()
{
    if (ItemData)
    {
        OnFutureItemInvalidated.Broadcast(ItemData->ItemInstanceID);
    }
    Destroy();
}