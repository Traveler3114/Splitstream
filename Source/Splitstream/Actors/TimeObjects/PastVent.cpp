#include "PastVent.h"
#include "Net/UnrealNetwork.h"

APastVent::APastVent()
{
}

void APastVent::BeginPlay()
{
    Super::BeginPlay();
}

void APastVent::Interact_Implementation(AActor* Interactor)
{
    Super::Interact_Implementation(Interactor);
}

void APastVent::OnRep_OpenState()
{
    OnVentStateChanged.Broadcast(bIsOpen);
    if (bIsOpen)
        OpenVent();
    else
        CloseVent();
}

void APastVent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void APastVent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}