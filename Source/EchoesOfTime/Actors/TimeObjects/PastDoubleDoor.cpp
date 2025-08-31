#include "PastDoubleDoor.h"
#include "LockPickingSystem/LockPickComponent.h"
#include "Net/UnrealNetwork.h"

APastDoubleDoor::APastDoubleDoor()
{
    LockPickComponent = CreateDefaultSubobject<ULockPickComponent>(TEXT("LockPickComponent"));
}

void APastDoubleDoor::BeginPlay()
{
    Super::BeginPlay();

    ULockPickComponent* LockPickComp = FindComponentByClass<ULockPickComponent>();
    if (LockPickComp)
    {
        LockPickComp->OnUnlock.AddDynamic(this, &APastDoubleDoor::OnLockUnlocked);
    }
}

void APastDoubleDoor::Interact_Implementation(AActor* Interactor)
{
    if (bIsLocked)
        return;

    Super::Interact_Implementation(Interactor);

    if (HasAuthority())
    {
        OnDoubleDoorStateChanged.Broadcast(bIsOpen);
    }
}

void APastDoubleDoor::OnRep_IsOpen()
{
    OnDoubleDoorStateChanged.Broadcast(bIsOpen);
    if (bIsOpen)
        OpenDoor();
    else
        CloseDoor();
}

void APastDoubleDoor::OnLockUnlocked()
{
    bIsLocked = false;
    bIsOpen = true;
    OnRep_IsOpen();
}

void APastDoubleDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APastDoubleDoor, bIsOpen);
    DOREPLIFETIME(APastDoubleDoor, bIsLocked);
}

void APastDoubleDoor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (LockPickComponent)
    {
        LockPickComponent->OnUnlock.RemoveDynamic(this, &APastDoubleDoor::OnLockUnlocked);
    }
    Super::EndPlay(EndPlayReason);
}