#include "PastPowerGenerator.h"
#include "ActorComponents/SearchComponent.h"

APastPowerGenerator::APastPowerGenerator()
{
}

void APastPowerGenerator::BeginPlay()
{
    Super::BeginPlay();
    if (SearchComponent)
    {
        SearchComponent->OnSearchComplete.AddDynamic(this, &APastPowerGenerator::OnSearchCompletedEvent);
    }
}

void APastPowerGenerator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (SearchComponent)
    {
        SearchComponent->OnSearchComplete.RemoveDynamic(this, &APastPowerGenerator::OnSearchCompletedEvent);
    }
    Super::EndPlay(EndPlayReason);
}

void APastPowerGenerator::Interact_Implementation(AActor* Interactor)
{
    Super::Interact_Implementation(Interactor);
}

void APastPowerGenerator::OnSearchCompletedEvent()
{
    OnGeneratorCompleted.Broadcast(true);
}

bool APastPowerGenerator::IsCompleted() const
{
    return SearchComponent && SearchComponent->bSearched;
}