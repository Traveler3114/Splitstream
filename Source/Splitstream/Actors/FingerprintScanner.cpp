// FingerprintScanner.cpp

#include "FingerprintScanner.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Net/UnrealNetwork.h"
#include "Interfaces/IUnlockable.h"
#include "Characters/CivilianCharacter.h"

AFingerprintScanner::AFingerprintScanner()
{
    PrimaryActorTick.bCanEverTick = false;

    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(DefaultSceneRoot);

    LinkedActor = nullptr;
}

void AFingerprintScanner::BeginPlay()
{
    Super::BeginPlay();
}

bool AFingerprintScanner::IsCorrectItem_Implementation(const FInventorySlot& Slot) const
{
    if (!Slot.ItemAsset || Slot.ItemAsset->ItemType != EItemType::Fingerprint)
        return false;

    ACivilianCharacter* OwnerCivilian = Slot.OwnerCivilian;
    if (!OwnerCivilian)
        return false;

    return OwnerCivilian->GetClass() == RequiredCivilianClass.Get();
}

void AFingerprintScanner::Interact_Implementation(AActor* Interactor)
{
    if (!HasAuthority())
        return;

    if (!Interactor || !LinkedActor)
        return;

    // Optionally, call interface unlock if LinkedActor supports it
    if (LinkedActor->GetClass()->ImplementsInterface(UUnlockable::StaticClass()))
    {
        IUnlockable::Execute_UnlockWithAccess(LinkedActor, Interactor);
    }
}

void AFingerprintScanner::SetHighlighted_Implementation(bool bHighlight)
{
    if (Mesh)
    {
        Mesh->SetRenderCustomDepth(bHighlight);
        Mesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}