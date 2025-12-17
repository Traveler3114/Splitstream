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
        OnDoubleDoorStateChanged.Broadcast(bIsOpen, OpenDirection);
    }
}

void APastDoubleDoor::OnRep_IsOpen()
{
    OnDoubleDoorStateChanged.Broadcast(bIsOpen, OpenDirection);
    if (bIsOpen)
        OpenDoor(OpenDirection);
    else
        CloseDoor(OpenDirection);
}

void APastDoubleDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void APastDoubleDoor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}