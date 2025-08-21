#include "PastDoor.h"
#include "Net/UnrealNetwork.h"

APastDoor::APastDoor()
{
    StaticMesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh1"));
    StaticMesh1->SetupAttachment(SceneRoot);

    StaticMesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh2"));
    StaticMesh2->SetupAttachment(SceneRoot);

    StaticMesh3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh3"));
    StaticMesh3->SetupAttachment(SceneRoot);
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