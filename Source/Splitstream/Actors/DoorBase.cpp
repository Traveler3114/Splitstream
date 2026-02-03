// DoorBase.cpp

#include "DoorBase.h"
#include "ActorComponents/LockPickComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
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

    ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
	ArrowComp->SetupAttachment(RootComponent);

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

    if (bIsLocked)
        return;

    if (HasAuthority())
    {
        OpenDirection = ComputeOpenDirection(Interactor);
        bIsOpen = !bIsOpen;
        OnRep_IsOpen();
    }
}

int32 ADoorBase::ComputeOpenDirection(AActor* ReferenceActor) const
{
    if (!ReferenceActor)
    {
        return 1;
    }
    FVector DoorForward = ArrowComp->GetForwardVector();
    FVector ToActor = ReferenceActor->GetActorLocation() - GetActorLocation();
    //FVector ToActor = ReferenceActor->GetActorLocation() - OpenDirectionArrow->GetComponentLocation();
    ToActor.Normalize();
    float Dot = FVector::DotProduct(DoorForward, ToActor);
    int32 Dir = (Dot < 0.f) ? 1 : -1;
    return Dir;
}

void ADoorBase::CancelInteract_Implementation(AActor* Interactor)
{
    if (bIsLocked && LockPickComponent)
    {
        LockPickComponent->CancelInteract(Interactor);
    }
}

bool ADoorBase::IsProgressiveInteract_Implementation()
{
    return (LockPickComponent && bIsLocked && !LockPickComponent->IsUnlocked());
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
        OpenDoor(OpenDirection);  // pass direction!
    else
        CloseDoor(OpenDirection);
}

void ADoorBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ADoorBase, bIsOpen);
    DOREPLIFETIME(ADoorBase, bIsLocked);
}

void ADoorBase::UnlockWithAccess_Implementation(AActor* Interactor)
{
    if (HasAuthority())
    {
        bIsOpen = true;
        OnRep_IsOpen();
    }
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
    if (OtherActor && OtherActor->IsA(AAICharacter::StaticClass()))
    {
        ForceOpenDoorForGuard(OtherActor);
    }
}

void ADoorBase::OnGuardOpenEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!bAutoOpenForGuards)
        return;
    if (OtherActor && OtherActor->IsA(AAICharacter::StaticClass()))
    {
        // Check if any other guards or civilians are still overlapping
        TArray<AActor*> Overlapping;
        GuardOpenTrigger->GetOverlappingActors(Overlapping, AGuardCharacter::StaticClass());
        GuardOpenTrigger->GetOverlappingActors(Overlapping, ACivilianCharacter::StaticClass());
        if (Overlapping.Num() == 0)
        {
            ForceCloseDoorForGuard(OtherActor);
        }
    }
}

void ADoorBase::ForceOpenDoorForGuard(AActor* GuardActor)
{
    if (!bIsOpen)
    {
        OpenDirection = ComputeOpenDirection(GuardActor);
        bIsOpen = true;
        OnRep_IsOpen();
    }
}

void ADoorBase::ForceCloseDoorForGuard(AActor* GuardActor)
{
    if (bIsOpen)
    {
        OpenDirection = ComputeOpenDirection(GuardActor);
        bIsOpen = false;
        OnRep_IsOpen();
    }
}