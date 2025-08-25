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
    ULockPickComponent* RealLockPickComponent = FindComponentByClass<ULockPickComponent>();
    if (RealLockPickComponent)
    {
        RealLockPickComponent->OnUnlock.AddDynamic(this, &APastDoor::OnLockUnlocked);
        if (HasAuthority()) {
            UE_LOG(LogTemp, Warning, TEXT("SERVER: PastDoor::BeginPlay bound OnUnlock to %s [LockPickComp=%p] (Owner=%s)"),
                *GetName(), RealLockPickComponent, RealLockPickComponent->GetOwner() ? *RealLockPickComponent->GetOwner()->GetName() : TEXT("None"));
        }
        else {
            UE_LOG(LogTemp, Warning, TEXT("CLIENT: PastDoor::BeginPlay bound OnUnlock to %s [LockPickComp=%p] (Owner=%s)"),
                *GetName(), RealLockPickComponent, RealLockPickComponent->GetOwner() ? *RealLockPickComponent->GetOwner()->GetName() : TEXT("None"));
        }
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
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Red, FString::Printf(TEXT("OnRep_IsOpen: bIsOpen=%d"), bIsOpen));
    if (bIsOpen)
        OpenDoor();
    else
        CloseDoor();
}

// When lockpicking succeeds, call this!
void APastDoor::OnLockUnlocked()
{
    UE_LOG(LogTemp, Warning, TEXT("%s: OnLockUnlocked called! HasAuthority: %d"), *GetName(), HasAuthority());
    if (!HasAuthority()) return; // Only the server should unlock/open the door!
    UE_LOG(LogTemp, Warning, TEXT("%s: OnLockUnlocked called! HasAuthority: %d [This=%p]"), *GetName(), HasAuthority(), this);
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