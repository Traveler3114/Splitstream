#include "FutureDoubleDoor.h"
#include "Actors/TimeObjects/PastDoubleDoor.h"
#include "Net/UnrealNetwork.h"

AFutureDoubleDoor::AFutureDoubleDoor()
{
}

void AFutureDoubleDoor::BeginPlay()
{
    Super::BeginPlay();

    if (PastDoubleDoor.IsValid())
    {
        PastDoubleDoor->OnDoubleDoorStateChanged.AddDynamic(this, &AFutureDoubleDoor::HandlePastDoubleDoorStateChanged);
    }
    else if (PastDoubleDoor.ToSoftObjectPath().IsValid())
    {
        APastDoubleDoor* LoadedPastDoubleDoor = Cast<APastDoubleDoor>(PastDoubleDoor.LoadSynchronous());
        if (LoadedPastDoubleDoor)
        {
            LoadedPastDoubleDoor->OnDoubleDoorStateChanged.AddDynamic(this, &AFutureDoubleDoor::HandlePastDoubleDoorStateChanged);
        }
    }
}

void AFutureDoubleDoor::Interact_Implementation(AActor* Interactor)
{
    Super::Interact_Implementation(Interactor);
}

void AFutureDoubleDoor::HandlePastDoubleDoorStateChanged(bool bPastIsOpen)
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

void AFutureDoubleDoor::OnRep_IsOpen()
{
    if (bIsOpen)
        OpenDoor();
    else
        CloseDoor();
}

void AFutureDoubleDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AFutureDoubleDoor, bIsOpen);
}

void AFutureDoubleDoor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    APastDoubleDoor* BoundPastDoubleDoor = nullptr;
    if (PastDoubleDoor.IsValid())
    {
        BoundPastDoubleDoor = PastDoubleDoor.Get();
    }
    else if (PastDoubleDoor.ToSoftObjectPath().IsValid())
    {
        BoundPastDoubleDoor = Cast<APastDoubleDoor>(PastDoubleDoor.LoadSynchronous());
    }
    if (BoundPastDoubleDoor)
    {
        BoundPastDoubleDoor->OnDoubleDoorStateChanged.RemoveDynamic(this, &AFutureDoubleDoor::HandlePastDoubleDoorStateChanged);
    }

    Super::EndPlay(EndPlayReason);
}