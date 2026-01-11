#include "ItemBase.h"
#include "GameplayTagContainer.h"
#include "Actors/ItemPickup.h"
#include "Actors/TimeObjects/PastItemPickup.h"
#include "Actors/TimeObjects/FutureItemPickup.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemInterface.h"
#include "ActorComponents/InventoryComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"


void UItemBase::OnEquipped(AActor* Instigator)
{
    
}

void UItemBase::OnUnequipped(AActor* Instigator)
{
    
}

void UItemBase::OnAddedToInventory(AActor* Instigator)
{
    
}

void UItemBase::OnRemovedFromInventory(AActor* Instigator)
{
    
}

// Pseudocode for ItemBase.cpp

void UItemBase::OnDropped(AActor* Instigator, FGuid ItemInstanceID, FVector DropLocation)
{
    if (!Instigator) return;
    UWorld* World = Instigator->GetWorld();
    if (!World) return;

    UClass* PickupClass = ItemPickupToSpawn ? ItemPickupToSpawn.Get() : AItemPickup::StaticClass();

    FVector SpawnLocation = bEnablePhysicsOnDrop ? Instigator->GetActorLocation() : DropLocation;
    FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
    AItemPickup* Pickup = World->SpawnActorDeferred<AItemPickup>(PickupClass, SpawnTransform);
    if (Pickup)
    {
        Pickup->ItemData = this;
        Pickup->ItemInstanceID = ItemInstanceID;
        UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);
        if (bEnablePhysicsOnDrop && Pickup->OverrideMeshComp)
        {
            Pickup->OverrideMeshComp->SetSimulatePhysics(true);
            FVector ForwardVector = Instigator->GetActorForwardVector();
            FVector Impulse = ForwardVector * DropImpulseStrength;
            Pickup->OverrideMeshComp->AddImpulse(Impulse, NAME_None, true);
        }
    }
}

void UItemBase::OnDroppedWithTeam(AActor* Instigator, FGuid ItemInstanceID, FGameplayTag TeamTag, FVector DropLocation)
{
    if (!Instigator) return;
    UWorld* World = Instigator->GetWorld();
    if (!World) return;
    FVector SpawnLocation = bEnablePhysicsOnDrop ? Instigator->GetActorLocation() : DropLocation;
    SpawnLocation.Z = FMath::Max(SpawnLocation.Z, 0.0f);
    FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

    static FGameplayTag PastTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Past"));
    static FGameplayTag FutureTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Future"));

    if (TeamTag == PastTag)
    {
        UClass* PickupClass = PastItemPickupToSpawn ? PastItemPickupToSpawn.Get() : APastItemPickup::StaticClass();
        APastItemPickup* Pickup = World->SpawnActorDeferred<APastItemPickup>(PickupClass, SpawnTransform);
        if (Pickup)
        {
            Pickup->ItemData = this;
            Pickup->ItemInstanceID = ItemInstanceID;
            UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);
            if (bEnablePhysicsOnDrop && Pickup->OverrideMeshComp)
            {
                Pickup->OverrideMeshComp->SetSimulatePhysics(true);
                FVector ForwardVector = Instigator->GetActorForwardVector();
                FVector Impulse = ForwardVector * DropImpulseStrength;
                Pickup->OverrideMeshComp->AddImpulse(Impulse, NAME_None, true);
            }
        }
    }
    else if (TeamTag == FutureTag)
    {
        UClass* PickupClass = FutureItemPickupToSpawn ? FutureItemPickupToSpawn.Get() : AFutureItemPickup::StaticClass();
        AFutureItemPickup* Pickup = World->SpawnActorDeferred<AFutureItemPickup>(PickupClass, SpawnTransform);
        if (Pickup)
        {
            Pickup->ItemData = this;
            Pickup->ItemInstanceID = ItemInstanceID;
            UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);
            if (bEnablePhysicsOnDrop && Pickup->OverrideMeshComp)
            {
                Pickup->OverrideMeshComp->SetSimulatePhysics(true);
                FVector ForwardVector = Instigator->GetActorForwardVector();
                FVector Impulse = ForwardVector * DropImpulseStrength;
                Pickup->OverrideMeshComp->AddImpulse(Impulse, NAME_None, true);
            }
        }
    }
    else
    {
        UClass* PickupClass = ItemPickupToSpawn ? ItemPickupToSpawn.Get() : AItemPickup::StaticClass();
        AItemPickup* Pickup = World->SpawnActorDeferred<AItemPickup>(PickupClass, SpawnTransform);
        if (Pickup)
        {
            Pickup->ItemData = this;
            Pickup->ItemInstanceID = ItemInstanceID;
            UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);
            if (bEnablePhysicsOnDrop && Pickup->OverrideMeshComp)
            {
                Pickup->OverrideMeshComp->SetSimulatePhysics(true);
                FVector ForwardVector = Instigator->GetActorForwardVector();
                FVector Impulse = ForwardVector * DropImpulseStrength;
                Pickup->OverrideMeshComp->AddImpulse(Impulse, NAME_None, true);
            }
        }
    }
}


void UItemBase::OnUsed(AActor* Instigator, FGuid ItemInstanceID)
{
    if (bRemoveFromInventoryOnUse && Instigator)
    {
        UInventoryComponent* Inventory = Instigator->FindComponentByClass<UInventoryComponent>();
        if (Inventory)
        {
            Inventory->RemoveItemByInstanceID(ItemInstanceID);
        }
    }
}

void UItemBase::GrantEffectsTo(AActor* Instigator, TArray<FActiveGameplayEffectHandle>& OutHandles) const
{
    OutHandles.Empty();
    if (GrantedGameplayEffects.Num() > 0)
    {
        IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(Instigator);
        if (AbilityInterface)
        {
            UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent();
            if (ASC)
            {
                for (TSubclassOf<UGameplayEffect> EffectClass : GrantedGameplayEffects)
                {
                    if (EffectClass)
                    {
                        FActiveGameplayEffectHandle Handle =
                            ASC->ApplyGameplayEffectToSelf(EffectClass->GetDefaultObject<UGameplayEffect>(), 1.0f, ASC->MakeEffectContext());
                        OutHandles.Add(Handle);
                    }
                }
            }
        }
    }
}

void UItemBase::GrantAbilitiesTo(AActor* Instigator, TArray<FGameplayAbilitySpecHandle>& OutHandles) const
{
    OutHandles.Empty();
    if (AbilitySet)
    {
        IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(Instigator);
        if (AbilityInterface)
        {
            UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent();
            if (ASC)
            {
                for (const FAbilityInputSetEntry& Entry : AbilitySet->Abilities)
                {
                    if (!Entry.AbilityClass) continue;
                    FGameplayAbilitySpec Spec(Entry.AbilityClass, Entry.AbilityLevel, 0);
                    Spec.GetDynamicSpecSourceTags().AddTag(Entry.InputTag);
                    FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
                    OutHandles.Add(Handle);
                }
            }
        }
    }
}

void UItemBase::RemoveGrantedEffectsFrom(AActor* Instigator, const TArray<FActiveGameplayEffectHandle>& Handles) const
{
    if (!Handles.Num()) return;
    IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(Instigator);
    if (!AbilityInterface) return;

    UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent();
    if (!ASC) return;

    for (const FActiveGameplayEffectHandle& Handle : Handles)
    {
        ASC->RemoveActiveGameplayEffect(Handle);
    }
}

void UItemBase::RemoveGrantedAbilitiesFrom(AActor* Instigator, const TArray<FGameplayAbilitySpecHandle>& Handles) const
{
    if (!Handles.Num()) return;
    IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(Instigator);
    if (!AbilityInterface) return;

    UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent();
    if (!ASC) return;

    for (const FGameplayAbilitySpecHandle& Handle : Handles)
    {
        ASC->ClearAbility(Handle);
    }
}