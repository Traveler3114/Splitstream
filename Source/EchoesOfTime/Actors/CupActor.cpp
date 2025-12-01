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
        UE_LOG(LogTemp, Error, TEXT("OnSearchComplete: SearchComponent is null."));
        return;
    }

    // Null check for LastInteractor
    AActor* LastInteractor = SearchComponent->LastInteractor.Get();
    if (!LastInteractor) {
        UE_LOG(LogTemp, Error, TEXT("OnSearchComplete: LastInteractor is null."));
        return;
    }

    UInventoryComponent* Inventory = LastInteractor->FindComponentByClass<UInventoryComponent>();
    if (!Inventory) {
        UE_LOG(LogTemp, Error, TEXT("OnSearchComplete: Inventory is null."));
        return;
    }

    UItemBase* Fingerprint = Cast<UItemBase>(StaticLoadObject(
        UItemBase::StaticClass(),
        nullptr,
        TEXT("/Game/DataAssets/Items/DA_Fingerprint.DA_Fingerprint")
    ));
    if (!Fingerprint) {
        UE_LOG(LogTemp, Error, TEXT("OnSearchComplete: Failed to load Fingerprint item asset."));
        return;
    }

    Fingerprint->OwnerCivilian = LinkedCivilian;
    FGuid NewInstanceID = FGuid::NewGuid();
    bool bAdded = Inventory->AddItem(Fingerprint, NewInstanceID);

    if (!bAdded) {
        UE_LOG(LogTemp, Warning, TEXT("OnSearchComplete: AddItem failed."));
    }
}