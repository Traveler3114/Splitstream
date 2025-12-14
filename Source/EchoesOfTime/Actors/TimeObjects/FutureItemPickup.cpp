#include "FutureItemPickup.h"
#include "PastItemPickup.h"
#include "ActorComponents/InventoryComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "DefaultPlayerState.h"
#include "GameFramework/PlayerController.h"

FFutureItemInvalidated AFutureItemPickup::OnFutureItemInvalidated;

AFutureItemPickup::AFutureItemPickup()
{
    OnFutureItemInvalidated.AddUObject(this, &AFutureItemPickup::HandleInvalidation);
}

void AFutureItemPickup::BeginPlay()
{
    Super::BeginPlay();
}

void AFutureItemPickup::Interact_Implementation(AActor* Interactor)
{
    if (!ItemData || !Interactor) return;

    UInventoryComponent* Inventory = Interactor->FindComponentByClass<UInventoryComponent>();
    if (Inventory)
    {
        Inventory->RegisterFutureInstance(ItemInstanceID);

        if (Inventory->AddItem(ItemData, ItemInstanceID))
        {
            Destroy();
        }
    }
}

void AFutureItemPickup::HandleInvalidation(FGuid InvalidatedID)
{
    if (ItemInstanceID == InvalidatedID)
    {
        Destroy();
    }
}


bool AFutureItemPickup::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
    // RealViewer is usually the PlayerController
    const APlayerController* PC = Cast<APlayerController>(RealViewer);
    if (!PC) return false;

    const ADefaultPlayerState* PS = PC->GetPlayerState<ADefaultPlayerState>();
    if (PS && PS->GetTeamName().Equals(TEXT("Future"), ESearchCase::IgnoreCase))
    {
        return true;
    }
    return false;
}