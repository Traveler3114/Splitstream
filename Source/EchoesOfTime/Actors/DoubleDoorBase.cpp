#include "DoubleDoorBase.h"
#include "ActorComponents/LockPickComponent.h"
#include "Components/BoxComponent.h"
#include "Characters/GuardCharacter.h"
#include "Net/UnrealNetwork.h"

ADoubleDoorBase::ADoubleDoorBase()
{
    bReplicates = true;
    SetReplicateMovement(true);
    NetUpdateFrequency = 10.f; // Double doors don't change often

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;
    SceneRoot->SetIsReplicated(true);

    DoorRightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorRightMesh"));
    DoorRightMesh->SetupAttachment(SceneRoot);
    DoorRightMesh->SetIsReplicated(true);
    DoorRightMesh->SetCanEverAffectNavigation(false);

    DoorLeftMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorLeftMesh"));
    DoorLeftMesh->SetupAttachment(SceneRoot);
    DoorLeftMesh->SetIsReplicated(true);
    DoorLeftMesh->SetCanEverAffectNavigation(false);

    GuardOpenTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("GuardOpenTrigger"));
    GuardOpenTrigger->SetupAttachment(RootComponent);
    GuardOpenTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GuardOpenTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    GuardOpenTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ADoubleDoorBase::BeginPlay()
{
    Super::BeginPlay();

    LockPickComponent = FindComponentByClass<ULockPickComponent>();
    if (LockPickComponent)
    {
        LockPickComponent->OnUnlock.AddDynamic(this, &ADoubleDoorBase::OnLockUnlocked);
    }
    GuardOpenTrigger->OnComponentBeginOverlap.AddDynamic(this, &ADoubleDoorBase::OnGuardOpenBeginOverlap);
    GuardOpenTrigger->OnComponentEndOverlap.AddDynamic(this, &ADoubleDoorBase::OnGuardOpenEndOverlap);
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


void ADoubleDoorBase::CancelInteract_Implementation(AActor* Interactor)
{
    if (bIsLocked && LockPickComponent)
    {
        LockPickComponent->CancelInteract(Interactor);
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
    if (DoorLeftMesh && DoorRightMesh)
    {
        DoorLeftMesh->SetRenderCustomDepth(bHighlight);
        DoorLeftMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
        DoorRightMesh->SetRenderCustomDepth(bHighlight);
        DoorRightMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}

// --- Guard Auto Open/Close Logic ---

void ADoubleDoorBase::OnGuardOpenBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bAutoOpenForGuards)
        return;
    if (OtherActor && OtherActor->IsA(AGuardCharacter::StaticClass()))
    {
        ForceOpenDoorForGuard();
    }
}

void ADoubleDoorBase::OnGuardOpenEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!bAutoOpenForGuards)
        return;
    if (OtherActor && OtherActor->IsA(AGuardCharacter::StaticClass()))
    {
        // Check if any other guards are still overlapping
        TArray<AActor*> Overlapping;
        GuardOpenTrigger->GetOverlappingActors(Overlapping, AGuardCharacter::StaticClass());
        if (Overlapping.Num() == 0)
        {
            ForceCloseDoorForGuard();
        }
    }
}

void ADoubleDoorBase::ForceOpenDoorForGuard()
{
    if (!bIsOpen)
    {
        bIsOpen = true;
        OnRep_IsOpen();
    }
    // Do NOT touch bIsLocked or bRequiresKeycard
}

void ADoubleDoorBase::ForceCloseDoorForGuard()
{
    if (bIsOpen)
    {
        bIsOpen = false;
        OnRep_IsOpen();
    }
    // Do NOT touch bIsLocked or bRequiresKeycard
}