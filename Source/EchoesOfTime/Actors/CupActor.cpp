#include "CupActor.h"
#include "ActorComponents/SearchComponent.h"
#include "ActorComponents/InventoryComponent.h"
#include "DataAssets/ItemBase.h"

ACupActor::ACupActor()
{
    CupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CupMesh"));
    CupMesh->SetupAttachment(DefaultSceneRoot);
}


void ACupActor::SetHighlighted_Implementation(bool bHighlight)
{
    if (CupMesh && SearchComponent)
    {
        if (SearchComponent->bSearched)
        {
            CupMesh->SetRenderCustomDepth(false);
            CupMesh->CustomDepthStencilValue = 0;
        }
        else
        {
            CupMesh->SetRenderCustomDepth(bHighlight);
            CupMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
        }
    }
}

void ACupActor::Interact_Implementation(AActor* Interactor)
{
    if (SearchComponent)
    {
        SearchComponent->Interact(Interactor);
    }
}

void ACupActor::CancelInteract_Implementation(AActor* Interactor)
{
    if (SearchComponent)
    {
        SearchComponent->CancelInteract(Interactor);
    }
}

void ACupActor::OnSearchComplete()
{
    Super::OnSearchComplete();

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

    UItemBase* Fingerprint = Cast<UItemBase>(StaticLoadObject(
        UItemBase::StaticClass(),
        nullptr,
        TEXT("/Game/DataAssets/Items/DA_Fingerprint.DA_Fingerprint")
    ));
    if (!Fingerprint) {
        return;
    }

    Fingerprint->OwnerCivilian = LinkedCivilian;
    FGuid NewInstanceID = FGuid::NewGuid();
    bool bAdded = Inventory->AddItem(Fingerprint, NewInstanceID);
}