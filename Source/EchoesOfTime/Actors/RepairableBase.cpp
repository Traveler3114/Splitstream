#include "RepairableBase.h"

ARepairableBase::ARepairableBase()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ARepairableBase::RequestRepair(AActor* RepairInstigator)
{
    // By default, just instantly "repair" (child classes can override)
    // (Maybe play VFX/SFX, reset destroyed/searched status, etc.)
}

float ARepairableBase::GetRepairTime() const
{
    return RepairTime;
}