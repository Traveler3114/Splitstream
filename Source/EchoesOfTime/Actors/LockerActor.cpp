// Fill out your copyright notice in the Description page of Project Settings.


#include "LockerActor.h"
#include "ActorComponents/LockPickComponent.h"
#include "Components/TextRenderComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ALockerActor::ALockerActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    SceneRootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(SceneRootComp);

    LockerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LockerMesh"));
    LockerMesh->SetupAttachment(SceneRootComp);
    LockerMesh->SetIsReplicated(true);

    LockerDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LockerDoorMesh"));
    LockerDoorMesh->SetupAttachment(SceneRootComp);

	NameText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NameText"));
    NameText->SetupAttachment(SceneRootComp);

	LockPickComponent = CreateDefaultSubobject<ULockPickComponent>(TEXT("LockPickComponent"));
	LockPickComponent->SetIsReplicated(true);
}

// Called when the game starts or when spawned
void ALockerActor::BeginPlay()
{
	Super::BeginPlay();
    OnRep_StaffName();
    if(LockPickComponent)
    {
        LockPickComponent->OnUnlock.AddDynamic(this, &ALockerActor::OpenLocker);
	}
}

void ALockerActor::OnRep_IsOpen()
{
    if (bIsOpen)
        OpenLocker();
    else
        CloseLocker();
}

void ALockerActor::Interact_Implementation(AActor* Interactor)
{
    if (LockPickComponent && !LockPickComponent->bUnlocked)
    {
        LockPickComponent->Interact(Interactor);
        return;
    }

    if (HasAuthority())
    {
        bIsOpen = !bIsOpen;
        OnRep_IsOpen();
    }
}
void ALockerActor::CancelInteract_Implementation(AActor* Interactor)
{
    if (LockPickComponent)
    {
        LockPickComponent->CancelInteract(Interactor);
    }
}
void ALockerActor::SetHighlighted_Implementation(bool bHighlight)
{
    if (LockerDoorMesh && LockerMesh)
    {
		LockerMesh->SetRenderCustomDepth(bHighlight);
		LockerMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
        LockerDoorMesh->SetRenderCustomDepth(bHighlight);
        LockerDoorMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}

void ALockerActor::SetStaffName(const FString& NewName)
{
    StaffName = NewName;
    OnRep_StaffName();
}

void ALockerActor::OnRep_StaffName()
{
    if (NameText)
        NameText->SetText(FText::FromString(StaffName));
}

void ALockerActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALockerActor, bIsOpen);
	DOREPLIFETIME(ALockerActor, StaffName);
}


