#include "FutureItemPickup.h"
#include "PastItemPickup.h"
#include "DrawDebugHelpers.h"

AFutureItemPickup::AFutureItemPickup()
{
}

void AFutureItemPickup::BeginPlay()
{
    Super::BeginPlay();

}


void AFutureItemPickup::OnPastItemPickedUp()
{
    Destroy();
}