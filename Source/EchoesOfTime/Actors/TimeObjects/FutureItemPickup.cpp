#include "FutureItemPickup.h"
#include "PastItemPickup.h"
#include "DrawDebugHelpers.h"

FFutureItemInvalidated AFutureItemPickup::OnFutureItemInvalidated;

AFutureItemPickup::AFutureItemPickup()
{
}

void AFutureItemPickup::BeginPlay()
{
    Super::BeginPlay();
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