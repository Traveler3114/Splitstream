#include "CupActor.h"
#include "ActorComponents/SearchComponent.h"
#include "ActorComponents/InventoryComponent.h"
#include "DataAssets/ItemBase.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

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

void ACupActor::OnSearchComplete()
{
    Super::OnSearchComplete();


    if (!HasAuthority()) return; // Ensure only server gives the item


    AActor* LastInteractor = SearchComponent->LastInteractor.Get();

    UInventoryComponent* Inventory = LastInteractor->FindComponentByClass<UInventoryComponent>();

    UItemBase* Fingerprint = Cast<UItemBase>(StaticLoadObject(
        UItemBase::StaticClass(),
        nullptr,
        TEXT("/Game/DataAssets/Items/DA_Fingerprint.DA_Fingerprint")
    ));
    Fingerprint->OwnerCivilian = LinkedCivilian;
    FGuid NewInstanceID = FGuid::NewGuid();
    bool bAdded = Inventory->AddItem(Fingerprint, NewInstanceID);
}