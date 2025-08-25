#include "PastDoor.h"
#include "LockPickingSystem/LockPickComponent.h"
#include "Net/UnrealNetwork.h"

APastDoor::APastDoor()
{
    LockPickComponent = CreateDefaultSubobject<ULockPickComponent>(TEXT("LockPickComponent"));
}
void APastDoor::BeginPlay()
{
    Super::BeginPlay();
    if (LockPickComponent)
    {
        LockPickComponent->OnUnlock.AddDynamic(this, &APastDoor::OnLockUnlocked);
    }
}
void APastDoor::Interact_Implementation(AActor* Interactor)
{
    // If locked, don't open; start lockpick (your character/ability system triggers lockpicking UI)
    if (bIsLocked)
    {
        // Optionally: feedback ("The door is locked") or trigger lockpicking here
        return;
    }

    // If not locked, interact as normal
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

// When lockpicking succeeds, call this!
void APastDoor::OnLockUnlocked()
{
    bIsLocked = false;
    bIsOpen = true;
    OnRep_IsOpen();
}

void APastDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APastDoor, bIsOpen);
    DOREPLIFETIME(APastDoor, bIsLocked); // if you want lock state to replicate
}