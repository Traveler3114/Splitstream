// Fill out your copyright notice in the Description page of Project Settings.


#include "SearchableActor.h"
#include "Components/SceneComponent.h"
#include "ActorComponents/SearchComponent.h"
#include "Characters/CivilianCharacter.h"
#include "ActorComponents/InventoryComponent.h"
#include "DataAssets/ItemBase.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASearchableActor::ASearchableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

    ActorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ActorMesh"));
    ActorMesh->SetupAttachment(DefaultSceneRoot);

    SearchComponent = CreateDefaultSubobject<USearchComponent>(TEXT("SearchComponent"));
    SearchComponent->SearchDuration = 5.f;
    SearchComponent->SetIsReplicated(true);

	bReplicates = true;
}

// Called when the game starts or when spawned
void ASearchableActor::BeginPlay()
{
	Super::BeginPlay();
    if (SearchComponent)
    {
        SearchComponent->OnSearchComplete.AddDynamic(this, &ASearchableActor::OnSearchComplete);
    }
}

void ASearchableActor::SetHighlighted_Implementation(bool bHighlight)
{
    if(!bIsActivatedForPlayer && bHighlight) return;

    if (ActorMesh && SearchComponent)
    {
        if (SearchComponent->bSearched)
        {
            ActorMesh->SetRenderCustomDepth(false);
            ActorMesh->CustomDepthStencilValue = 0;
        }
        else
        {
            ActorMesh->SetRenderCustomDepth(bHighlight);
            ActorMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
        }
    }
}

void ASearchableActor::Interact_Implementation(AActor* Interactor)
{
    if (ACivilianCharacter* Civilian = Cast<ACivilianCharacter>(Interactor))
    {
        bIsActivatedForPlayer = true;
		RewardItem->OwnerCivilian = Civilian;
        // Optionally show highlight, play SFX etc
        return;
    }
    else if (!bIsActivatedForPlayer)
    {
        // Block player from searching until a civilian has used
        return;
    }
    // Only hits here if player and search is activated
    if (SearchComponent)
        SearchComponent->Interact(Interactor);
}


void ASearchableActor::CancelInteract_Implementation(AActor* Interactor)
{
    if (SearchComponent)
    {
        SearchComponent->CancelInteract(Interactor);
    }
}

bool ASearchableActor::IsProgressiveInteract_Implementation()
{
    return true;
}

void ASearchableActor::OnSearchComplete()
{
    if (!bGivesItem) return;

	if (!RewardItem) return;

    if (!HasAuthority()) return; // Ensure only server gives the item

    // Null check for SearchComponent
    if (!SearchComponent) {
        return;
    }

    // Null check for LastInteractor
    AActor* LastInteractor = SearchComponent->LastInteractor.Get();
    if (!LastInteractor) {
        return;
    }

    UInventoryComponent* Inventory = LastInteractor->FindComponentByClass<UInventoryComponent>();
    if (!Inventory) {
        return;
    }

    //RewardItem->OwnerCivilian = LinkedCivilian;
    FGuid NewInstanceID = FGuid::NewGuid();
    bool bAdded = Inventory->AddItem(RewardItem, NewInstanceID);

    bIsActivatedForPlayer = false;
    SetHighlighted_Implementation(false);
}

void ASearchableActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASearchableActor, bIsActivatedForPlayer);
}


