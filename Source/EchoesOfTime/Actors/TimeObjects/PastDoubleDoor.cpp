#include "PastDoubleDoor.h"
#include "Net/UnrealNetwork.h"

APastDoubleDoor::APastDoubleDoor()
{
    // No LockPickComponent logic here!
}

void APastDoubleDoor::BeginPlay()
{
    Super::BeginPlay();
    // No LockPick logic here!
}

void APastDoubleDoor::Interact_Implementation(AActor* Interactor)
{
    Super::Interact_Implementation(Interactor);

    if (HasAuthority())
    {
        OnDoubleDoorStateChanged.Broadcast(bIsOpen);
    }
}

void APastDoubleDoor::OnRep_IsOpen()
{
    OnDoubleDoorStateChanged.Broadcast(bIsOpen);
    if (bIsOpen)
        OpenDoor();
    else
        CloseDoor();
}

void APastDoubleDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
void APastDoubleDoor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}