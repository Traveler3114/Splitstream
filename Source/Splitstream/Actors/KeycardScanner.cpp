#include "KeycardScanner.h"

AKeycardScanner::AKeycardScanner()
{
    PrimaryActorTick.bCanEverTick = false;
    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(DefaultSceneRoot);

    LinkedActor = nullptr;
    RequiredKeycardType = EItemType::KeycardL1; // Default
}

void AKeycardScanner::BeginPlay()
{
    Super::BeginPlay();
}

bool AKeycardScanner::IsCorrectItem_Implementation(UItemBase* Item) const
{
    // This is the ONLY place that checks if the item is the correct keycard.
    return Item && Item->ItemType == RequiredKeycardType;
}
void AKeycardScanner::Interact_Implementation(AActor* Interactor)
{
    if (!HasAuthority()) return;

    if (!Interactor || !LinkedActor)
        return;

    if (LinkedActor->GetClass()->ImplementsInterface(UKeycardUnlockable::StaticClass()))
    {
        IKeycardUnlockable::Execute_UnlockWithKeycard(LinkedActor, Interactor);
    }
}

void AKeycardScanner::SetHighlighted_Implementation(bool bHighlight)
{
    if (Mesh)
    {
        Mesh->SetRenderCustomDepth(bHighlight);
        Mesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}