#include "CupActor.h"
#include "ActorComponents/SearchComponent.h"
#include "ActorComponents/InventoryComponent.h"
#include "DataAssets/ItemBase.h"
#include "DataAssets/FingerprintItem.h"
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


void ACupActor::OnSearchComplete()
{
    Super::OnSearchComplete();

    if (!HasAuthority()) return; // Ensure only server gives the item

    if (!SearchComponent) return;
    AActor* LastInteractor = SearchComponent->LastInteractor.Get();
    if (!LastInteractor) return;

    UInventoryComponent* Inventory = LastInteractor->FindComponentByClass<UInventoryComponent>();
    if (Inventory)
    {
        UFingerprintItem* Fingerprint = Cast<UFingerprintItem>(StaticLoadObject(
            UFingerprintItem::StaticClass(),
            nullptr,
            TEXT("/Game/DataAssets/Items/DA_Fingerprint.DA_Fingerprint")
        ));
        if (Fingerprint)
        {
            Fingerprint->OwnerCivilian = LinkedCivilian;
            FGuid NewInstanceID = FGuid::NewGuid();
            Inventory->AddItem(Fingerprint, NewInstanceID);
        }
    }
}