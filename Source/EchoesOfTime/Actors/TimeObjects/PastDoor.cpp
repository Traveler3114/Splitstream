#include "PastDoor.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h" // For debug messages

APastDoor::APastDoor()
{
    // No LockPickComponent logic here!
}

void APastDoor::BeginPlay()
{
    Super::BeginPlay();
    // No LockPick logic here!
}

void APastDoor::Interact_Implementation(AActor* Interactor)
{
    Super::Interact_Implementation(Interactor);

    if (HasAuthority())
    {
        OnDoorStateChanged.Broadcast(bIsOpen);
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
}
void APastDoor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}