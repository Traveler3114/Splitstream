#include "FutureItemPickup.h"
#include "PastItemPickup.h"
#include "InventorySystem/InventoryComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"

FFutureItemInvalidated AFutureItemPickup::OnFutureItemInvalidated;

AFutureItemPickup::AFutureItemPickup()
{
    OnFutureItemInvalidated.AddUObject(this, &AFutureItemPickup::HandleInvalidation);
}

void AFutureItemPickup::BeginPlay()
{
    Super::BeginPlay();

    // Restore LinkedPastItem if it's null
    if (!LinkedPastItem && ItemData)
    {
        for (TActorIterator<APastItemPickup> It(GetWorld()); It; ++It)
        {
            if (It->ItemData == ItemData && It->ItemInstanceID == ItemInstanceID)
            {
                LinkedPastItem = *It;
                break;
            }
        }
    }
}

void AFutureItemPickup::Interact_Implementation(AActor* Interactor)
{
    if (!ItemData || !Interactor) return;

    UInventoryComponent* Inventory = Interactor->FindComponentByClass<UInventoryComponent>();
    if (Inventory)
    {
        Inventory->RegisterFutureInstance(ItemInstanceID);

        if (Inventory->AddItem(ItemData, ItemInstanceID))
        {
            Destroy();
        }
    }
}

void AFutureItemPickup::HandleInvalidation(FGuid InvalidatedID)
{
    if (ItemInstanceID == InvalidatedID)
    {
        Destroy();
    }
}