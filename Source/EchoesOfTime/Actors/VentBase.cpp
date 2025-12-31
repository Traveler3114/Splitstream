// Fill out your copyright notice in the Description page of Project Settings.


#include "VentBase.h"
#include "ActorComponents/SearchComponent.h"
#include "ActorComponents/DetectionComponent.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"


// Sets default values
AVentBase::AVentBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	VentBaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VentBaseMesh"));
	VentBaseMesh->SetupAttachment(SceneRoot);

    SearchComponent = CreateDefaultSubobject<USearchComponent>(TEXT("SearchComponent"));
    SearchComponent->SetIsReplicated(true);

    DetectionComponent = CreateDefaultSubobject<UDetectionComponent>(TEXT("DetectionComponent"));
    DetectionComponent->SetIsReplicated(true);

    StimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));

}

// Called when the game starts or when spawned
void AVentBase::BeginPlay()
{
	Super::BeginPlay();
    if (SearchComponent)
    {
        SearchComponent->OnSearchComplete.AddDynamic(this, &AVentBase::OnSearchComplete);
	}
}

void AVentBase::OnDetected_Implementation(AActor* Detector)
{
    if (DetectionComponent && !(DetectionComponent->bDetectionInProgress) && !(DetectionComponent->bFullyDetected)&& bIsOpen) DetectionComponent->StartDetection(Detector);
}
void AVentBase::OnLost_Implementation(AActor* Detector)
{
    if (DetectionComponent) DetectionComponent->StopDetection(Detector);
}

void AVentBase::OnForceDetectionEnd_Implementation(AActor* Detector)
{
    if (DetectionComponent) DetectionComponent->ForceImmediateDetectionEnd(Detector);
}

void AVentBase::Interact_Implementation(AActor* Interactor)
{
    if (SearchComponent)
        SearchComponent->Interact(Interactor);
}

void AVentBase::CancelInteract_Implementation(AActor* Interactor)
{
    if (SearchComponent)
        SearchComponent->CancelInteract(Interactor);
}

bool AVentBase::IsProgressiveInteract_Implementation()
{
    return true;
}

void AVentBase::SetHighlighted_Implementation(bool bHighlight)
{
    if (VentBaseMesh)
    {
        VentBaseMesh->SetRenderCustomDepth(bHighlight);
        VentBaseMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}

void AVentBase::OnSearchComplete()
{
    if (HasAuthority())
    {
        bIsOpen = !bIsOpen;
		OnRep_OpenState();
    }
}

void AVentBase::OnRep_OpenState()
{
    if (bIsOpen)
    {
        OpenVent();
    }
    else
    {
        CloseVent();
    }
}

void AVentBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AVentBase, bIsOpen);
}


