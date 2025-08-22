#include "FutureItemPickup.h"
#include "PastItemPickup.h"

AFutureItemPickup::AFutureItemPickup() {}

void AFutureItemPickup::OnPastItemPickedUp()
{
    Destroy();
}