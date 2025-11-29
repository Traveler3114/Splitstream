#include "DoorBase.h"
#include "ActorComponents/LockPickComponent.h"
#include "Components/BoxComponent.h"
#include "Characters/GuardCharacter.h"
#include "Characters/CivilianCharacter.h"
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

    GuardOpenTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("GuardOpenTrigger"));
    GuardOpenTrigger->SetupAttachment(RootComponent);
    GuardOpenTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GuardOpenTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    GuardOpenTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ADoorBase::BeginPlay()
{
    Super::BeginPlay();

    LockPickComponent = FindComponentByClass<ULockPickComponent>();
    if (LockPickComponent)
    {
        LockPickComponent->OnUnlock.AddDynamic(this, &ADoorBase::OnLockUnlocked);
    }
    GuardOpenTrigger->OnComponentBeginOverlap.AddDynamic(this, &ADoorBase::OnGuardOpenBeginOverlap);
    GuardOpenTrigger->OnComponentEndOverlap.AddDynamic(this, &ADoorBase::OnGuardOpenEndOverlap);
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

// --- Guard Auto Open/Close Logic ---

void ADoorBase::OnGuardOpenBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bAutoOpenForGuards)
        return;
    if (OtherActor && (OtherActor->IsA(AGuardCharacter::StaticClass()) || OtherActor->IsA(ACivilianCharacter::StaticClass())))
    {
        ForceOpenDoorForGuard();
    }
}

void ADoorBase::OnGuardOpenEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!bAutoOpenForGuards)
        return;
    if (OtherActor && OtherActor->IsA(AGuardCharacter::StaticClass()))
    {
        // Check if any other guards or civilians are still overlapping
        TArray<AActor*> Overlapping;
        GuardOpenTrigger->GetOverlappingActors(Overlapping, AGuardCharacter::StaticClass());
        GuardOpenTrigger->GetOverlappingActors(Overlapping, ACivilianCharacter::StaticClass());
        if (Overlapping.Num() == 0)
        {
            ForceCloseDoorForGuard();
        }
    }
}

void ADoorBase::ForceOpenDoorForGuard()
{
    if (!bIsOpen)
    {
        bIsOpen = true;
        OnRep_IsOpen();
    }
    // Do NOT touch bIsLocked or bRequiresKeycard
}

void ADoorBase::ForceCloseDoorForGuard()
{
    if (bIsOpen)
    {
        bIsOpen = false;
        OnRep_IsOpen();
    }
    // Do NOT touch bIsLocked or bRequiresKeycard
}