#include "DoorBase.h"
#include "Net/UnrealNetwork.h"

ADoorBase::ADoorBase()
{
    bReplicates = true;
    SetReplicateMovement(true);

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;
	SceneRoot->SetIsReplicated(true);

    DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
    DoorMesh->SetupAttachment(SceneRoot);
	DoorMesh->SetIsReplicated(true);
    DoorFrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrameMesh"));
    DoorFrameMesh->SetupAttachment(SceneRoot);
	DoorFrameMesh->SetIsReplicated(true);
}

void ADoorBase::Interact_Implementation(AActor* Interactor)
{
    // If requires keycard, block direct opening by the player
    if (bRequiresKeycard)
    {
        // Optional: Play denial sound or feedback
        return;
    }

    if (HasAuthority())
    {
        bIsOpen = !bIsOpen;
        if (bIsOpen)
            OpenDoor();
        else
            CloseDoor();
    }
}

void ADoorBase::OnRep_IsOpen()
{
    if (bIsOpen)
        OpenDoor();
    else
        CloseDoor();
}

void ADoorBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ADoorBase, bIsOpen);
}