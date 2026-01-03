#include "ItemPickup.h"
#include "ActorComponents/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ActorComponents/SearchComponent.h"
#include "ActorComponents/DetectionComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
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
    if (StimuliSourceComponent)
    {
        StimuliSourceComponent->bAutoRegister = true;
        // Register to be sensed by sight by default (you can add other senses if needed)
        StimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
    }
}

void AItemPickup::BeginPlay()
{
    Super::BeginPlay();

    // Find optional search component and bind delegate
    SearchComp = FindComponentByClass<USearchComponent>();
    if (SearchComp)
    {
        SearchComp->OnSearchComplete.AddDynamic(this, &AItemPickup::OnSearchComplete);
    }
    if (StimuliSourceComponent)
    {
        StimuliSourceComponent->bAutoRegister = true;
        // Register to be sensed by sight by default (you can add other senses if needed)
        StimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
    }
}

void AItemPickup::InitFromItemData(UItemBase* InItemData, FGuid InInstanceID)
{
    ItemData = InItemData;
    ItemInstanceID = InInstanceID;
}

void AItemPickup::RefreshMeshFromItemData()
{

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
        OnItemPickedUp.Broadcast(Interactor, ItemData);
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
}
#endif

void AItemPickup::OnDetected_Implementation(AActor* Detector)
{
    if (ItemData && ItemData->bAlertGuardsWhenSeen && Detector && DetectionComponent)
    {
        if (!DetectionComponent->IsDetectionInProgress(Detector) && !DetectionComponent->IsFullyDetected(Detector))
        {
            DetectionComponent->StartDetection(Detector);
        }
    }
}

void AItemPickup::OnLost_Implementation(AActor* Detector)
{
    if (DetectionComponent && DetectionComponent->IsDetectionInProgress(Detector) && !DetectionComponent->IsFullyDetected(Detector)) DetectionComponent->StopDetection(Detector);
}

void AItemPickup::OnForceDetectionEnd_Implementation(AActor* Detector)
{
    if (DetectionComponent) DetectionComponent->ForceImmediateDetectionEnd(Detector);
}