// Fill out your copyright notice in the Description page of Project Settings.


#include "SearchableActor.h"
#include "Components/SceneComponent.h"
#include "ActorComponents/SearchComponent.h"
#include "Characters/CivilianCharacter.h"
#include "ActorComponents/InventoryComponent.h"
#include "DataAssets/ItemBase.h"

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
	SearchComponent->bSearched = true;

	bReplicates = true;
}

// Called when the game starts or when spawned
void ASearchableActor::BeginPlay()
{
	Super::BeginPlay();
    if (SearchComponent)
    {
        if (HasAuthority()) // Only set on server
            SearchComponent->bSearched = true;
        SearchComponent->OnSearchComplete.AddDynamic(this, &ASearchableActor::OnSearchComplete);
    }
}

void ASearchableActor::SetHighlighted_Implementation(bool bHighlight)
{
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
        // NPC re-enables the search for players
        if (SearchComponent)
            SearchComponent->bSearched = false;
        PendingOwnerCivilian = Civilian;
        return;
    }

    // Only allow players to search if not already searched (i.e. after NPC unlock)
    if (SearchComponent && !SearchComponent->bSearched)
    {
        SearchComponent->Interact(Interactor);
    }
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
    if (!bGivesItem || !RewardItem || !HasAuthority() || !SearchComponent) return;
    AActor* LastInteractor = SearchComponent->LastInteractor.Get();
    if (!LastInteractor) return;

    UInventoryComponent* Inventory = LastInteractor->FindComponentByClass<UInventoryComponent>();
    if (!Inventory) return;

    FGuid NewInstanceID = FGuid::NewGuid();
    bool bAdded = Inventory->AddItem(RewardItem, NewInstanceID);
    if (bAdded)
    {
        for (FInventorySlot& Slot : Inventory->Slots)
        {
            if (Slot.ItemAsset == RewardItem && Slot.ItemInstanceID == NewInstanceID)
            {
                Slot.OwnerCivilian = PendingOwnerCivilian; // Only set on server
                break;
            }
        }
    }

    SetHighlighted_Implementation(false);
}

