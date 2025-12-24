#include "ItemPickup.h"
#include "ActorComponents/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ActorComponents/SearchComponent.h"
#include "ActorComponents/DetectionComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Engine/Engine.h"

AItemPickup::AItemPickup()
{
    bReplicates = true;
    SetReplicateMovement(true);

    USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = SceneRoot;

    OverrideMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OverrideMeshComp"));
    OverrideMeshComp->SetupAttachment(SceneRoot);
    OverrideMeshComp->SetIsReplicated(true);

    DetectionComponent = CreateDefaultSubobject<UDetectionComponent>(TEXT("DetectionComponent"));
    DetectionComponent->SetIsReplicated(true);

    ItemData = nullptr;
    ItemInstanceID.Invalidate();

    StimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));
}

void AItemPickup::BeginPlay()
{
    Super::BeginPlay();

    if (OverrideMeshComp && !OverrideMeshComp->GetStaticMesh() && ItemData && ItemData->ItemMesh)
    {
        OverrideMeshComp->SetStaticMesh(ItemData->ItemMesh);
        OverrideMeshComp->SetWorldScale3D(ItemData->PickupMeshScale);
    }

    // Find optional search component and bind delegate
    SearchComp = FindComponentByClass<USearchComponent>();
    if (SearchComp)
    {
        SearchComp->OnSearchComplete.AddDynamic(this, &AItemPickup::OnSearchComplete);
    }

}

void AItemPickup::InitFromItemData(UItemBase* InItemData, FGuid InInstanceID)
{
    ItemData = InItemData;
    ItemInstanceID = InInstanceID;
    if (OverrideMeshComp && !OverrideMeshComp->GetStaticMesh() && ItemData && ItemData->ItemMesh)
    {
        OverrideMeshComp->SetStaticMesh(ItemData->ItemMesh);
        OverrideMeshComp->SetWorldScale3D(ItemData->PickupMeshScale);
    }
}

void AItemPickup::RefreshMeshFromItemData()
{
    if (OverrideMeshComp && ItemData && ItemData->ItemMesh)
    {
        OverrideMeshComp->SetStaticMesh(ItemData->ItemMesh);
        OverrideMeshComp->SetWorldScale3D(ItemData->PickupMeshScale);
    }
}

void AItemPickup::OnSearchComplete()
{
    // Just use LastInteractor from the SearchComponent (already set in its Interact)
    AActor* Interactor = SearchComp ? SearchComp->LastInteractor.Get() : nullptr;
    TryPickup(Interactor);
}

void AItemPickup::Interact_Implementation(AActor* Interactor)
{
    if (SearchComp)
    {
        // This will internally set LastInteractor in SearchComp and start search
        SearchComp->Interact(Interactor);
        // Do not continue with pickup logic here; OnSearchComplete will handle it if search required
        return;
    }

    TryPickup(Interactor);
}

void AItemPickup::CancelInteract_Implementation(AActor* Interactor)
{
    if (SearchComp)
    {
        SearchComp->CancelInteract(Interactor);
    }
}

bool AItemPickup::IsProgressiveInteract_Implementation()
{
    return SearchComp != nullptr;
}

void AItemPickup::TryPickup(AActor* Interactor)
{
    if (!HasAuthority()) return;
    if (!ItemData || !Interactor) return;

    UInventoryComponent* Inventory = Interactor->FindComponentByClass<UInventoryComponent>();
    if (Inventory && Inventory->AddItem(ItemData, ItemInstanceID))
    {
        OnPickedUp.Broadcast(Interactor, ItemData);
        Destroy();
    }
}

void AItemPickup::SetHighlighted_Implementation(bool bHighlight)
{
    if (OverrideMeshComp)
    {
        OverrideMeshComp->SetRenderCustomDepth(bHighlight);
        OverrideMeshComp->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}

#if WITH_EDITOR
void AItemPickup::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (!ItemInstanceID.IsValid())
    {
        ItemInstanceID = FGuid::NewGuid();
    }

    if (OverrideMeshComp && !OverrideMeshComp->GetStaticMesh() && ItemData && ItemData->ItemMesh)
    {
        OverrideMeshComp->SetStaticMesh(ItemData->ItemMesh);
        OverrideMeshComp->SetWorldScale3D(ItemData->PickupMeshScale);
    }
}
#endif

void AItemPickup::OnDetected_Implementation(AActor* Detector)
{
    if (ItemData && ItemData->bAlertGuardsWhenSeen && Detector)
    {
        DetectionComponent->StartDetection(Detector);
    }
}

void AItemPickup::OnLost_Implementation(AActor* Detector)
{
    if (ItemData && ItemData->bAlertGuardsWhenSeen && Detector)
    {
        DetectionComponent->StopDetection(Detector);
    }
}