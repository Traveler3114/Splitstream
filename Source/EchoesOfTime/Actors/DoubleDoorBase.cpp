#include "DoubleDoorBase.h"
#include "ActorComponents/LockPickComponent.h"
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

void ADoubleDoorBase::BeginPlay()
{
    Super::BeginPlay();

    LockPickComponent = FindComponentByClass<ULockPickComponent>();
    if (LockPickComponent)
    {
        LockPickComponent->OnUnlock.AddDynamic(this, &ADoubleDoorBase::OnLockUnlocked);
    }
}

void ADoubleDoorBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (LockPickComponent)
    {
        LockPickComponent->OnUnlock.RemoveDynamic(this, &ADoubleDoorBase::OnLockUnlocked);
    }
    Super::EndPlay(EndPlayReason);
}

void ADoubleDoorBase::Interact_Implementation(AActor* Interactor)
{
    if (bIsLocked && LockPickComponent)
    {
        LockPickComponent->Interact(Interactor);
        return;
    }

    if (bRequiresKeycard)
    {
        return;
    }

    if (HasAuthority())
    {
        bIsOpen = !bIsOpen;
        OnRep_IsOpen();
    }
}

void ADoubleDoorBase::OnLockUnlocked()
{
    bIsLocked = false;
    bIsOpen = true;
    OnRep_IsOpen();
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
    DOREPLIFETIME(ADoubleDoorBase, bIsLocked);
}

void ADoubleDoorBase::UnlockWithKeycard_Implementation(AActor* Interactor)
{
    if (HasAuthority())
    {
        bIsOpen = true;
        OnRep_IsOpen();
    }
}

bool ADoubleDoorBase::RequiresKeycard_Implementation() const
{
    return bRequiresKeycard;
}


void ADoubleDoorBase::SetHighlighted_Implementation(bool bHighlight)
{
    if (DoorLeftMesh&&DoorRightMesh)
    {
        DoorLeftMesh->SetRenderCustomDepth(bHighlight);
        DoorLeftMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
        DoorRightMesh->SetRenderCustomDepth(bHighlight);
        DoorRightMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}