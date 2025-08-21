#include "PastDoor.h"
#include "Net/UnrealNetwork.h"

APastDoor::APastDoor()
{

}

void APastDoor::Interact_Implementation(AActor* Interactor)
{
    Super::Interact_Implementation(Interactor);
    if (HasAuthority())
    {
        OnDoorStateChanged.Broadcast(bIsOpen); // Custom logic: broadcast state
    }
}

void APastDoor::OnRep_IsOpen()
{
    OnDoorStateChanged.Broadcast(bIsOpen);
    if (bIsOpen)
        OpenDoor();
    else
        CloseDoor();
}

void APastDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APastDoor, bIsOpen);
}