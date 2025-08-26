#include "PastDoor.h"
#include "LockPickingSystem/LockPickComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h" // For debug messages

APastDoor::APastDoor()
{
    LockPickComponent = CreateDefaultSubobject<ULockPickComponent>(TEXT("LockPickComponent"));
}

void APastDoor::BeginPlay()
{
    Super::BeginPlay();

    ULockPickComponent* LockPickComp = FindComponentByClass<ULockPickComponent>();
    if (LockPickComp)
    {
        LockPickComp->OnUnlock.AddDynamic(this, &APastDoor::OnLockUnlocked);

    }
}

void APastDoor::Interact_Implementation(AActor* Interactor)
{
    if (bIsLocked)
    {
        return;
    }

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
    DOREPLIFETIME(APastDoor, bIsLocked);
}