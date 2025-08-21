#include "FutureDoor.h"
#include "Actors/TimeObjects/PastDoor.h"
#include "Net/UnrealNetwork.h"

AFutureDoor::AFutureDoor()
{
    StaticMesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh1"));
    StaticMesh1->SetupAttachment(SceneRoot);

    StaticMesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh2"));
    StaticMesh2->SetupAttachment(SceneRoot);

    StaticMesh3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh3"));
    StaticMesh3->SetupAttachment(SceneRoot);
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

    // Only proceed if not locked (bRequiresKeycard is false)
    if (bRequiresKeycard)
        return;

    if (HasAuthority())
    {
        bIsOpen = !bIsOpen;
        if (bIsOpen)
            OpenDoor();
        else
            CloseDoor();
    }
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