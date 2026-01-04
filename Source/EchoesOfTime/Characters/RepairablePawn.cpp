#include "RepairablePawn.h"

ARepairablePawn::ARepairablePawn()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ARepairablePawn::RequestPawnRepair(AActor* RepairInstigator)
{
    // By default, just instantly "repair" (child classes can override)
    // (Maybe play VFX/SFX, reset destroyed/searched status, etc.)
}

float ARepairablePawn::GetRepairTime() const
{
    return RepairTime;
}