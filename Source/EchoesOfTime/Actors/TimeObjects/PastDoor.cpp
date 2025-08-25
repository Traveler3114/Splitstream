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

    UE_LOG(LogTemp, Warning, TEXT("PastDoor::BeginPlay called on %s [HasAuthority=%d]"), *GetName(), HasAuthority());
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Orange, FString::Printf(TEXT("PastDoor::BeginPlay %s [HasAuthority=%d]"), *GetName(), HasAuthority()));

    ULockPickComponent* LockPickComp = FindComponentByClass<ULockPickComponent>();
    if (LockPickComp)
    {
        LockPickComp->OnUnlock.AddDynamic(this, &APastDoor::OnLockUnlocked);

        UE_LOG(LogTemp, Warning, TEXT("%s: Bound OnUnlock to LockPickComp=%p (Owner=%s) [HasAuthority=%d]"),
            *GetName(), LockPickComp, LockPickComp->GetOwner() ? *LockPickComp->GetOwner()->GetName() : TEXT("None"), HasAuthority());
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Orange, FString::Printf(TEXT("%s: Bound OnUnlock to LockPickComp=%p (Owner=%s) [HasAuth=%d]"), *GetName(), LockPickComp, LockPickComp->GetOwner() ? *LockPickComp->GetOwner()->GetName() : TEXT("None"), HasAuthority()));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("%s: No LockPickComponent found!"), *GetName());
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("%s: No LockPickComponent found!"), *GetName()));
    }
}

void APastDoor::Interact_Implementation(AActor* Interactor)
{
    UE_LOG(LogTemp, Warning, TEXT("%s: Interact_Implementation called. bIsLocked=%d, HasAuth=%d"), *GetName(), bIsLocked, HasAuthority());
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("%s: Interact called. Locked=%d"), *GetName(), bIsLocked));
    if (bIsLocked)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Door is locked. Interact ends."), *GetName());
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s: Door is locked!"), *GetName()));
        return;
    }

    Super::Interact_Implementation(Interactor);

    if (HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Broadcasting OnDoorStateChanged (bIsOpen=%d)"), *GetName(), bIsOpen);
        OnDoorStateChanged.Broadcast(bIsOpen);
    }
}

void APastDoor::OnRep_IsOpen()
{
    UE_LOG(LogTemp, Warning, TEXT("%s: OnRep_IsOpen called. bIsOpen=%d"), *GetName(), bIsOpen);
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString::Printf(TEXT("%s: OnRep_IsOpen: bIsOpen=%d"), *GetName(), bIsOpen));
    OnDoorStateChanged.Broadcast(bIsOpen);
    if (bIsOpen)
        OpenDoor();
    else
        CloseDoor();
}

void APastDoor::OnLockUnlocked()
{
    UE_LOG(LogTemp, Error, TEXT("%s: OnLockUnlocked called! HasAuthority: %d [This=%p]"), *GetName(), HasAuthority(), this);
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("%s: OnLockUnlocked! HasAuth=%d"), *GetName(), HasAuthority()));

    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: OnLockUnlocked called on client, ignoring."), *GetName());
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s: OnLockUnlocked client, ignoring."), *GetName()));
        return;
    }

    bIsLocked = false;
    bIsOpen = true;
    UE_LOG(LogTemp, Warning, TEXT("%s: Door UNLOCKED and OPENED (bIsOpen=%d, bIsLocked=%d)"), *GetName(), bIsOpen, bIsLocked);
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("%s: Door UNLOCKED & OPEN!"), *GetName()));

    OnRep_IsOpen();
}

void APastDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APastDoor, bIsOpen);
    DOREPLIFETIME(APastDoor, bIsLocked);
}