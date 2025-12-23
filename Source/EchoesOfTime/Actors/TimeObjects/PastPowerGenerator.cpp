#include "PastPowerGenerator.h"
#include "ActorComponents/SearchComponent.h"

APastPowerGenerator::APastPowerGenerator()
{
}

void APastPowerGenerator::BeginPlay()
{
    Super::BeginPlay();
}

void APastPowerGenerator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (SearchComponent)
    {
        SearchComponent->OnSearchComplete.RemoveDynamic(this, &APastPowerGenerator::OnSearchComplete);
    }
    Super::EndPlay(EndPlayReason);
}

void APastPowerGenerator::Interact_Implementation(AActor* Interactor)
{
    Super::Interact_Implementation(Interactor);
}

void APastPowerGenerator::OnSearchComplete()
{
	Super::OnSearchComplete();
    OnGeneratorCompleted.Broadcast(SearchComponent->bSearched);
}
