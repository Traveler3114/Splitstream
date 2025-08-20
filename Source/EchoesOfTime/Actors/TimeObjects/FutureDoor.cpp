// Fill out your copyright notice in the Description page of Project Settings.


#include "FutureDoor.h"
#include "Actors/TimeObjects/PastDoor.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AFutureDoor::AFutureDoor()
{

    bReplicates = true;
    SetReplicateMovement(true); // if needed
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

void AFutureDoor::BeginPlay()
{
    Super::BeginPlay();

    if (PastDoor.IsValid())
    {
        // Already resolved, safe to bind
        PastDoor->OnDoorStateChanged.AddDynamic(this, &AFutureDoor::HandlePastDoorStateChanged);
    }
    else if (PastDoor.ToSoftObjectPath().IsValid())
    {
        // Force-load / resolve soft ref
        APastDoor* LoadedPastDoor = Cast<APastDoor>(PastDoor.LoadSynchronous());
        if (LoadedPastDoor)
        {
            LoadedPastDoor->OnDoorStateChanged.AddDynamic(this, &AFutureDoor::HandlePastDoorStateChanged);
        }
    }
}


void AFutureDoor::Interact_Implementation(AActor* Interactor)
{
    if (bIsOpen)
    {
        CloseDoor();
        bIsOpen = false;
    }
    else
    {
        OpenDoor();
        bIsOpen = true;
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


void AFutureDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AFutureDoor, bIsOpen);
}

void AFutureDoor::OnRep_IsOpen()
{
    // Optionally trigger open/close door visuals here
    if (bIsOpen)
        OpenDoor();
    else
        CloseDoor();
}