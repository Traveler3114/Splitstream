#include "FutureVent.h"
#include "PastVent.h"
#include "Net/UnrealNetwork.h"

AFutureVent::AFutureVent()
{
}

void AFutureVent::BeginPlay()
{
    Super::BeginPlay();

    if (PastVent.IsValid())
    {
        PastVent->OnVentStateChanged.AddDynamic(this, &AFutureVent::HandlePastVentStateChanged);
    }
    else if (PastVent.ToSoftObjectPath().IsValid())
    {
        APastVent* LoadedPastVent = Cast<APastVent>(PastVent.LoadSynchronous());
        if (LoadedPastVent)
        {
            LoadedPastVent->OnVentStateChanged.AddDynamic(this, &AFutureVent::HandlePastVentStateChanged);
        }
    }
}

void AFutureVent::Interact_Implementation(AActor* Interactor)
{
    if (!bIsInteractable)
    {
        return;
    }
    Super::Interact_Implementation(Interactor);
}

void AFutureVent::CancelInteract_Implementation(AActor* Interactor)
{
    if (!bIsInteractable)
    {
        return;
    }
    Super::CancelInteract_Implementation(Interactor);
}

void AFutureVent::SetHighlighted_Implementation(bool bHighlight)
{
    if (!bIsInteractable)
    {
        return;
    }
    Super::SetHighlighted_Implementation(bHighlight);
}

void AFutureVent::HandlePastVentStateChanged(bool bPastIsOpen)
{
    if (bPastIsOpen)
    {
        OpenVent();
        bIsOpen = true;
    }
    else
    {
        CloseVent();
        bIsOpen = false;
    }
}

void AFutureVent::OnRep_OpenState()
{
    if (bIsOpen)
        OpenVent();
    else
        CloseVent();
}

void AFutureVent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AFutureVent, bIsOpen);
}

void AFutureVent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    APastVent* BoundPastVent = nullptr;
    if (PastVent.IsValid())
    {
        BoundPastVent = PastVent.Get();
    }
    else if (PastVent.ToSoftObjectPath().IsValid())
    {
        BoundPastVent = Cast<APastVent>(PastVent.LoadSynchronous());
    }
    if (BoundPastVent)
    {
        BoundPastVent->OnVentStateChanged.RemoveDynamic(this, &AFutureVent::HandlePastVentStateChanged);
    }

    Super::EndPlay(EndPlayReason);
}