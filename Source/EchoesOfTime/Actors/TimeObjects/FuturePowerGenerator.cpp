#include "FuturePowerGenerator.h"
#include "PastPowerGenerator.h"

AFuturePowerGenerator::AFuturePowerGenerator()
{
}

void AFuturePowerGenerator::BeginPlay()
{
    Super::BeginPlay();

    APastPowerGenerator* Past = nullptr;
    if (PastGenerator.IsValid())
    {
        Past = PastGenerator.Get();
    }
    else if (PastGenerator.ToSoftObjectPath().IsValid())
    {
        Past = Cast<APastPowerGenerator>(PastGenerator.LoadSynchronous());
    }

    if (Past)
    {
        Past->OnGeneratorCompleted.AddDynamic(this, &AFuturePowerGenerator::HandlePastGeneratorCompleted);
        // Optionally: If already completed in the past, update self state at start
        if (Past->IsCompleted())
        {
            HandlePastGeneratorCompleted(true);
        }
    }
}

void AFuturePowerGenerator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    APastPowerGenerator* Past = nullptr;
    if (PastGenerator.IsValid())
    {
        Past = PastGenerator.Get();
    }
    else if (PastGenerator.ToSoftObjectPath().IsValid())
    {
        Past = Cast<APastPowerGenerator>(PastGenerator.LoadSynchronous());
    }
    if (Past)
    {
        Past->OnGeneratorCompleted.RemoveDynamic(this, &AFuturePowerGenerator::HandlePastGeneratorCompleted);
    }
    Super::EndPlay(EndPlayReason);
}

void AFuturePowerGenerator::HandlePastGeneratorCompleted(bool bCompleted)
{
    // Optionally trigger your own visuals, UI, etc!
    // Could also set a bCompleted/bSearched state if you want a local copy.
    if (bCompleted && SearchComponent && !SearchComponent->bSearched)
    {
        SearchComponent->bSearched = true;
        SearchComponent->OnSearchComplete.Broadcast();
    }
}

void AFuturePowerGenerator::Interact_Implementation(AActor* Interactor)
{
    Super::Interact_Implementation(Interactor);
}

void AFuturePowerGenerator::CancelInteract_Implementation(AActor* Interactor)
{
    Super::CancelInteract_Implementation(Interactor);
}

void AFuturePowerGenerator::SetHighlighted_Implementation(bool bHighlight)
{
    Super::SetHighlighted_Implementation(bHighlight);
}

bool AFuturePowerGenerator::IsCompleted() const
{
    return SearchComponent && SearchComponent->bSearched;
}