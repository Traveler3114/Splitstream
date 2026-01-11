#include "HackComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/SplitstreamGameplayTags.h"

UHackComponent::UHackComponent()
{
    SetIsReplicatedByDefault(true);
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.SetTickFunctionEnable(true);
    bHackingInProgress = false;
    bHacked = false;
    HackDuration = 10.f;
    HackElapsed = 0.f;
}

void UHackComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UHackComponent::StartHacking()
{
    if (bHackingInProgress || bHacked) return;
    if (GetOwnerRole() == ROLE_Authority)
    {
        MulticastResetHackElapsed();
    }
    bHackingInProgress = true;
    SetComponentTickEnabled(true);
}
void UHackComponent::CancelHacking()
{
    bHackingInProgress = false;
    SetComponentTickEnabled(false);
    if (GetOwnerRole() == ROLE_Authority)
    {
        MulticastResetHackElapsed();
    }
}
float UHackComponent::GetHackProgress() const
{
    if (!bHackingInProgress || HackDuration <= 0.f) return 0.f;
    return FMath::Clamp(HackElapsed / HackDuration, 0.f, 1.f);
}

void UHackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (bHackingInProgress)
    {
        HackElapsed += DeltaTime;
        if (HackElapsed >= HackDuration)
        {
            bHackingInProgress = false;
            bHacked = true;
            SetComponentTickEnabled(false);
            OnHackComplete.Broadcast();
        }
    }
}

void UHackComponent::OnRep_Hacked()
{
    if (bHacked)
    {
        OnHackComplete.Broadcast();
    }
}

void UHackComponent::MulticastResetHackElapsed_Implementation()
{
    HackElapsed = 0.f;
}

void UHackComponent::Interact(AActor* Interactor)
{
    if (bHacked || bHackingInProgress) return;

    // Fire gameplay event for GAS
    if (IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(Interactor))
    {
        if (UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent())
        {
            FGameplayEventData EventData;
            EventData.Instigator = Interactor;
            EventData.OptionalObject = GetOwner(); // This is the hackable actor, needed for GAS ability

            ASC->HandleGameplayEvent(
                TAG_Character_Ability_Hack,
                &EventData
            );
        }
    }
}
void UHackComponent::CancelInteract(AActor* Interactor)
{
    CancelHacking();
    if (Interactor)
    {
        // Get AbilitySystemComponent from the interactor
        if (IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(Interactor))
        {
            if (UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent())
            {
                FGameplayTagContainer CancelTags;
                CancelTags.AddTag(TAG_Character_Ability_Hack);
                ASC->CancelAbilities(&CancelTags);
            }
        }
    }
}

void UHackComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UHackComponent, bHacked);
    DOREPLIFETIME(UHackComponent, bHackingInProgress);
    // Replicate other relevant properties if needed
}