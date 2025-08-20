#include "PastDoor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

APastDoor::APastDoor()
{

    bReplicates = true;
    SetReplicateMovement(true); // if needed
    // Create Scene Root
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
    Door->SetupAttachment(SceneRoot);
    // Create and attach Static Meshes
    StaticMesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh1"));
    StaticMesh1->SetupAttachment(SceneRoot);

    StaticMesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh2"));
    StaticMesh2->SetupAttachment(SceneRoot);

    StaticMesh3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh3"));
    StaticMesh3->SetupAttachment(SceneRoot);


}

void APastDoor::Interact_Implementation(AActor* Interactor)
{
    if (HasAuthority())
    {
        bIsOpen = !bIsOpen; // This will trigger OnRep_IsOpen on clients
        OnDoorStateChanged.Broadcast(bIsOpen); // Server-side listeners
        if (bIsOpen)
            OpenDoor();
        else
            CloseDoor();
    }
}

void APastDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APastDoor, bIsOpen);
}

void APastDoor::OnRep_IsOpen()
{
    // Call the delegate so listeners are updated on all clients
    OnDoorStateChanged.Broadcast(bIsOpen);

    // Optionally trigger open/close door visuals here
    if (bIsOpen)
        OpenDoor();
    else
        CloseDoor();
}