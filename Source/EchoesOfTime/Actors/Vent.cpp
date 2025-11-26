// Fill out your copyright notice in the Description page of Project Settings.


#include "Vent.h"
#include "ActorComponents/SearchComponent.h"

// Sets default values
AVent::AVent()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	VentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VentMesh"));
	VentMesh->SetupAttachment(SceneRoot);

    SearchComponent = CreateDefaultSubobject<USearchComponent>(TEXT("SearchComponent"));
    SearchComponent->SetIsReplicated(true);
}

// Called when the game starts or when spawned
void AVent::BeginPlay()
{
	Super::BeginPlay();
    if (SearchComponent)
    {
        SearchComponent->OnSearchComplete.AddDynamic(this, &AVent::OnSearchComplete);
	}
	
}

void AVent::Interact_Implementation(AActor* Interactor)
{
    if (SearchComponent)
        SearchComponent->Interact(Interactor);
}

void AVent::SetHighlighted_Implementation(bool bHighlight)
{
    if (VentMesh)
    {
        VentMesh->SetRenderCustomDepth(bHighlight);
        VentMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}

void AVent::OnSearchComplete()
{
    if (HasAuthority())
    {
        bIsOpen = !bIsOpen;
		OnRep_OpenState();
    }
}

void AVent::OnRep_OpenState()
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

void AVent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AVent, bIsOpen);
}


