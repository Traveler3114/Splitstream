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
    if (bSearchingInProgress) return; // block concurrent searches

    if (bSearched && !bAllowMultipleSearches) return; // only block if already searched AND only one search allowed

    bSearchingInProgress = true;
    if (GetOwner()->HasAuthority())
    {
        MulticastResetSearchElapsed();
    }
    SetComponentTickEnabled(true);

    if (bAllowMultipleSearches)
        bSearched = false; // clear so future checks (UI, code) see the new search in progress
}


void USearchComponent::CancelSearching()
{
    bSearchingInProgress = false;
    SetComponentTickEnabled(false);
    if (GetOwner()->HasAuthority())
    {
        MulticastResetSearchElapsed();
    }
}

void USearchComponent::MulticastResetSearchElapsed_Implementation()
{
    SearchElapsed = 0.f;
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
            bSearched = !bSearched;
            SetComponentTickEnabled(false);
            OnSearchComplete.Broadcast();
        }
    }
}

void USearchComponent::OnRep_Searched()
{
    OnSearchComplete.Broadcast();
}


void USearchComponent::Interact(AActor* Interactor)
{
    if (GetOwner()->HasAuthority()) {
        LastInteractor = Interactor;
    }
    if ((bSearched || bSearchingInProgress) && !bAllowMultipleSearches) return;
    if (GetOwner()->HasAuthority())
    {
        LastInteractor = Interactor; // 'Interactor' MUST be the server pawn
    }
    // Fire gameplay event for GAS
    if (IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(Interactor))
    {
        if (UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent())
        {
            FGameplayEventData EventData;
            EventData.Instigator = Interactor;
            EventData.OptionalObject = GetOwner(); // This is the Searchable actor, needed for GAS ability

            ASC->HandleGameplayEvent(
                TAG_Character_Ability_Search,
                &EventData
            );
        }
    }
}

void USearchComponent::CancelInteract(AActor* Interactor)
{
    CancelSearching();
    if (Interactor)
    {
        // Get AbilitySystemComponent from the interactor
        if (IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(Interactor))
        {
            if (UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent())
            {
                FGameplayTagContainer CancelTags;
                CancelTags.AddTag(TAG_Character_Ability_Search);
                ASC->CancelAbilities(&CancelTags);
            }
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