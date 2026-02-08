#include "DoubleDoorBase.h"
#include "ActorComponents/LockPickComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "Characters/GuardCharacter.h"
#include "Characters/CivilianCharacter.h"
#include "Net/UnrealNetwork.h"

ADoubleDoorBase::ADoubleDoorBase()
{
    bReplicates = true;
    SetReplicateMovement(true);

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;
    SceneRoot->SetIsReplicated(true);

    ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
    ArrowComp->SetupAttachment(RootComponent);

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

    if (bIsLocked)
    {
        return;
    }

    if (HasAuthority())
    {
        OpenDirection = ComputeOpenDirection(Interactor);
        bIsOpen = !bIsOpen;
        OnRep_IsOpen();
    }
}

int32 ADoubleDoorBase::ComputeOpenDirection(AActor* ReferenceActor) const
{
    if (!ReferenceActor)
    {
        return 1;
    }

    FVector DoorForward = ArrowComp->GetForwardVector();
    FVector ToActor = ReferenceActor->GetActorLocation() - GetActorLocation();
    ToActor.Normalize();
    float Dot = FVector::DotProduct(DoorForward, ToActor);
    int32 Dir = (Dot < 0.f) ? 1 : -1;
    return Dir;
}

void ADoubleDoorBase::CancelInteract_Implementation(AActor* Interactor)
{
    if (bIsLocked && LockPickComponent)
    {
        LockPickComponent->CancelInteract(Interactor);
    }
}

bool ADoubleDoorBase::IsProgressiveInteract_Implementation()
{
    return (LockPickComponent && bIsLocked && !LockPickComponent->IsUnlocked());
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
        OpenDoor(OpenDirection);
    else
        CloseDoor(OpenDirection);
}

void ADoubleDoorBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ADoubleDoorBase, bIsOpen);
    DOREPLIFETIME(ADoubleDoorBase, bIsLocked);
}

void ADoubleDoorBase::UnlockWithAccess_Implementation(AActor* Interactor)
{
    if (HasAuthority())
    {
        bIsOpen = true;
        OnRep_IsOpen();
    }
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
    if (OtherActor && OtherActor->IsA(AAICharacter::StaticClass()))
    {
        ForceOpenDoorForGuard(OtherActor);
    }
}

void ADoubleDoorBase::OnGuardOpenEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!bAutoOpenForGuards)
        return;
    if (OtherActor && OtherActor->IsA(AAICharacter::StaticClass()))
    {
        // Check if any other guards or civilians are still overlapping
        TArray<AActor*> OverlappingGuards;
        TArray<AActor*> OverlappingCivilians;
        GuardOpenTrigger->GetOverlappingActors(OverlappingGuards, AGuardCharacter::StaticClass());
        GuardOpenTrigger->GetOverlappingActors(OverlappingCivilians, ACivilianCharacter::StaticClass());
        if (OverlappingGuards.Num() == 0 && OverlappingCivilians.Num() == 0)
        {
            ForceCloseDoorForGuard(OtherActor);
        }
    }
}

void ADoubleDoorBase::ForceOpenDoorForGuard(AActor* GuardActor)
{
    if (!bIsOpen)
    {
        OpenDirection = ComputeOpenDirection(GuardActor);
        bIsOpen = true;
        OnRep_IsOpen();
    }
}

void ADoubleDoorBase::ForceCloseDoorForGuard(AActor* GuardActor)
{
    if (bIsOpen)
    {
        OpenDirection = ComputeOpenDirection(GuardActor);
        bIsOpen = false;
        OnRep_IsOpen();
    }
}