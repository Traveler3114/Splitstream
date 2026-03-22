// AEnvironmentalObject.cpp
#include "EnvironmentalObject.h"
#include "Components/StaticMeshComponent.h"

AEnvironmentalObject::AEnvironmentalObject()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);
}

AEnvironmentalSlot* AEnvironmentalObject::GetAvailableSlot() const
{
    for (AEnvironmentalSlot* Slot : Slots)
    {
        if (Slot && Slot->IsAvailable())
            return Slot;
    }
    return nullptr;
}
