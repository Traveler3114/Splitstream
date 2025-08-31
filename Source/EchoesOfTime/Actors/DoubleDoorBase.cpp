#include "DoubleDoorBase.h"
#include "Net/UnrealNetwork.h"

ADoubleDoorBase::ADoubleDoorBase()
{
    bReplicates = true;
    SetReplicateMovement(true);

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;
    SceneRoot->SetIsReplicated(true);

    DoorRightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorRightMesh"));
    DoorRightMesh->SetupAttachment(SceneRoot);
    DoorRightMesh->SetIsReplicated(true);

    DoorLeftMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorLeftMesh"));
    DoorLeftMesh->SetupAttachment(SceneRoot);
    DoorLeftMesh->SetIsReplicated(true);
}

void ADoubleDoorBase::Interact_Implementation(AActor* Interactor)
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

void ADoubleDoorBase::OnRep_IsOpen()
{
    if (bIsOpen)
        OpenDoor();
    else
        CloseDoor();
}

void ADoubleDoorBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ADoubleDoorBase, bIsOpen);
}