#include "FutureDoor.h"
#include "Actors/TimeObjects/PastDoor.h"
#include "Net/UnrealNetwork.h"

AFutureDoor::AFutureDoor()
{
}

void AFutureDoor::BeginPlay()
{
    Super::BeginPlay();

    if (PastDoor.IsValid())
    {
        PastDoor->OnDoorStateChanged.AddDynamic(this, &AFutureDoor::HandlePastDoorStateChanged);
    }
    else if (PastDoor.ToSoftObjectPath().IsValid())
    {
        APastDoor* LoadedPastDoor = Cast<APastDoor>(PastDoor.LoadSynchronous());
        if (LoadedPastDoor)
        {
            LoadedPastDoor->OnDoorStateChanged.AddDynamic(this, &AFutureDoor::HandlePastDoorStateChanged);
        }
    }
}

void AFutureDoor::Interact_Implementation(AActor* Interactor)
{
    Super::Interact_Implementation(Interactor); // <--- Add this line
}

void AFutureDoor::HandlePastDoorStateChanged(bool bPastIsOpen)
{
    if (bPastIsOpen)
    {
        OpenDoor();
        bIsOpen = true;
    }
    else
    {
        CloseDoor();
        bIsOpen = false;
    }
}

void AFutureDoor::OnRep_IsOpen()
{
    if (bIsOpen)
        OpenDoor();
    else
        CloseDoor();
}

void AFutureDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AFutureDoor, bIsOpen);
}

void AFutureDoor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    APastDoor* BoundPastDoor = nullptr;
    if (PastDoor.IsValid())
    {
        BoundPastDoor = PastDoor.Get();
    }
    else if (PastDoor.ToSoftObjectPath().IsValid())
    {
        BoundPastDoor = Cast<APastDoor>(PastDoor.LoadSynchronous());
    }
    if (BoundPastDoor)
    {
        BoundPastDoor->OnDoorStateChanged.RemoveDynamic(this, &AFutureDoor::HandlePastDoorStateChanged);
    }

    Super::EndPlay(EndPlayReason);
}