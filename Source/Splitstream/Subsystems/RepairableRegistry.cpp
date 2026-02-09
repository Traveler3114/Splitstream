#include "RepairableRegistry.h"

void URepairableRegistry::RegisterRepairable(AActor* Actor)
{
    if (Actor && !Repairables.Contains(Actor))
    {
        Repairables.Add(Actor);
        OnRepairableRegistered.Broadcast(Actor);
    }
}

void URepairableRegistry::UnregisterRepairable(AActor* Actor)
{
    if (Repairables.Remove(Actor) > 0)
    {
        OnRepairableUnregistered.Broadcast(Actor);
    }
}