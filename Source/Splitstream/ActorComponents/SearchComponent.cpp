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
    if (bSearchingInProgress)
    {
        if (bAllowMultipleSearches)
        {
            if (GetOwner() && GetOwner()->HasAuthority())
            {
                MulticastResetSearchElapsed();
            }
            SearchElapsed = 0.f;
            SetComponentTickEnabled(true);
        }
        else
        {
            return;
        }
    }
    else
    {
        if (bSearched && !bAllowMultipleSearches)
        {
            return;
        }

        bSearchingInProgress = true;
        if (GetOwner() && GetOwner()->HasAuthority())
        {
            MulticastResetSearchElapsed();
        }
        SetComponentTickEnabled(true);
    }
}

void USearchComponent::CancelSearching()
{
    bSearchingInProgress = false;
    SetComponentTickEnabled(false);
    if (GetOwner() && GetOwner()->HasAuthority())
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
    float Progress = (!bSearchingInProgress || SearchDuration <= 0.f) ? 0.f
        : FMath::Clamp(SearchElapsed / SearchDuration, 0.f, 1.f);
    return Progress;
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
            if (bAllowMultipleSearches)
            {
                bSearched = !bSearched;
            }
            else
            {
                bSearched = true;
            }
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
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        LastInteractor = Interactor;
    }

    if ((bSearched || bSearchingInProgress) && !bAllowMultipleSearches)
    {
        return;
    }

    if (GetOwner() && GetOwner()->HasAuthority())
    {
        LastInteractor = Interactor;
    }

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

void USearchComponent::CancelInteract(AActor* Interactor)
{
    CancelSearching();
    if (Interactor)
    {
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
}