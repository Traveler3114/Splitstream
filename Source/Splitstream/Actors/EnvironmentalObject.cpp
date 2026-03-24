// AEnvironmentalObject.cpp
#include "EnvironmentalObject.h"
#include "Components/StaticMeshComponent.h"
#include "ActorComponents/SearchComponent.h"
#include "ActorComponents/InventoryComponent.h"
#include "AI/Characters/CivilianCharacter.h"
#include "DataAssets/ItemBase.h"

AEnvironmentalObject::AEnvironmentalObject()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);
}

void AEnvironmentalObject::BeginPlay()
{
    Super::BeginPlay();
    SearchComponent = FindComponentByClass<USearchComponent>();
    if (SearchComponent)
    {
        if (HasAuthority())
            SearchComponent->bSearched = true;

        SearchComponent->OnSearchComplete.AddDynamic(this, &AEnvironmentalObject::OnSearchComplete);
    }
}

// ── IInteractable ─────────────────────────────────────────────────────────────

void AEnvironmentalObject::Interact_Implementation(AActor* Interactor)
{
    if (!SearchComponent) return;

    // Civilian NPC unlocks the object for player searching
    if (ACivilianCharacter* Civilian = Cast<ACivilianCharacter>(Interactor))
    {
        if (SearchComponent)
            SearchComponent->bSearched = false;
        PendingOwnerCivilian = Civilian;
        return;
    }

    // Player searches the object
    if (SearchComponent && !SearchComponent->bSearched)
        SearchComponent->Interact(Interactor);
}

void AEnvironmentalObject::CancelInteract_Implementation(AActor* Interactor)
{
    if (SearchComponent)
        SearchComponent->CancelInteract(Interactor);
}

void AEnvironmentalObject::SetHighlighted_Implementation(bool bHighlight)
{
    if (!Mesh || !SearchComponent) return;

    if (SearchComponent->bSearched)
    {
        Mesh->SetRenderCustomDepth(false);
        Mesh->CustomDepthStencilValue = 0;
    }
    else
    {
        Mesh->SetRenderCustomDepth(bHighlight);
        Mesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}

// ── NPC Interaction ───────────────────────────────────────────────────────────

AEnvironmentalSlot* AEnvironmentalObject::GetAvailableSlot() const
{
    for (AEnvironmentalSlot* Slot : Slots)
    {
        if (Slot && Slot->IsAvailable())
            return Slot;
    }
    return nullptr;
}

// ── Player Search ─────────────────────────────────────────────────────────────

void AEnvironmentalObject::OnSearchComplete()
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
                Slot.OwnerCivilian = PendingOwnerCivilian;
                break;
            }
        }
    }

    SetHighlighted_Implementation(false);
}
