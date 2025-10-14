#include "DoorBase.h"
#include "ActorComponents/LockPickComponent.h"
#include "Components/BoxComponent.h"
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
    DoorMesh->SetCanEverAffectNavigation(false);
}

void ADoorBase::BeginPlay()
{
    Super::BeginPlay();

    LockPickComponent = FindComponentByClass<ULockPickComponent>();
    if (LockPickComponent)
    {
        LockPickComponent->OnUnlock.AddDynamic(this, &ADoorBase::OnLockUnlocked);
    }
}

void ADoorBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (LockPickComponent)
    {
        LockPickComponent->OnUnlock.RemoveDynamic(this, &ADoorBase::OnLockUnlocked);
    }
    Super::EndPlay(EndPlayReason);
}

void ADoorBase::Interact_Implementation(AActor* Interactor)
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

void ADoorBase::OnLockUnlocked()
{
    bIsLocked = false;
    bIsOpen = true;
    OnRep_IsOpen();
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
    DOREPLIFETIME(ADoorBase, bIsLocked);
}

void ADoorBase::UnlockWithKeycard_Implementation(AActor* Interactor)
{
    if (HasAuthority())
    {
        bIsOpen = true;
        OnRep_IsOpen();
    }
}

bool ADoorBase::RequiresKeycard_Implementation() const
{
    return bRequiresKeycard;
}


void ADoorBase::SetHighlighted_Implementation(bool bHighlight)
{
    if (DoorMesh)
    {
        DoorMesh->SetRenderCustomDepth(bHighlight);
        DoorMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}