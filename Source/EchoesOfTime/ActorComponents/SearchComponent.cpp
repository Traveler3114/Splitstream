#include "SearchComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/EOTGameplayTags.h"

USearchComponent::USearchComponent()
{
    SetIsReplicatedByDefault(true);
    PrimaryComponentTick.bCanEverTick = true;
    bSearchingInProgress = false;
    bSearched = false;
    SearchDuration = 10.f;
    SearchElapsed = 0.f;
}

void USearchComponent::BeginPlay()
{
    Super::BeginPlay();
}

void USearchComponent::StartSearching()
{
    if (bSearchingInProgress || bSearched) return;
    bSearchingInProgress = true;
    SearchElapsed = 0.f;
    SetComponentTickEnabled(true);
}


void USearchComponent::CancelSearching()
{
    bSearchingInProgress = false;
    SetComponentTickEnabled(false);
}

float USearchComponent::GetSearchProgress() const
{
    if (!bSearchingInProgress || SearchDuration <= 0.f) return 0.f;
    return FMath::Clamp(SearchElapsed / SearchDuration, 0.f, 1.f);
}

void USearchComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (bSearchingInProgress)
    {
        SearchElapsed += DeltaTime;
        if (SearchElapsed >= SearchDuration)
        {
            bSearchingInProgress = false;
            bSearched = true;
            SetComponentTickEnabled(false);
            OnSearchComplete.Broadcast();
        }
    }
}

void USearchComponent::OnRep_Searched()
{
    if (bSearched)
    {
        OnSearchComplete.Broadcast();
    }
}


void USearchComponent::Interact(AActor* Interactor)
{
    if (bSearched || bSearchingInProgress) return;

    // Only set LastInteractor if we have authority (server)
    if (GetOwner()->HasAuthority())
    {
        LastInteractor = Interactor; // On the server, Interactor should be the server's pawn
    }

    // Fire gameplay event for GAS (should work if this is called on the server)
    if (IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(Interactor))
    {
        if (UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent())
        {
            FGameplayEventData EventData;
            EventData.Instigator = Interactor;
            EventData.OptionalObject = GetOwner();

            ASC->HandleGameplayEvent(
                TAG_Character_Ability_Search,
                &EventData
            );
        }
    }
}

void USearchComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(USearchComponent, bSearched);
    DOREPLIFETIME(USearchComponent, bSearchingInProgress);
    // Replicate other relevant properties if needed
}