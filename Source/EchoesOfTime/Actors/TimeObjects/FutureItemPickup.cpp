#include "FutureItemPickup.h"
#include "PastItemPickup.h"
#include "DrawDebugHelpers.h"

AFutureItemPickup::AFutureItemPickup()
{
}

void AFutureItemPickup::BeginPlay()
{
    Super::BeginPlay();
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("FutureItemPickup"));
    }
}


void AFutureItemPickup::OnPastItemPickedUp()
{
    Destroy();
}