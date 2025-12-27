#include "ItemBase.h"
#include "GameplayTagContainer.h"
#include "Actors/ItemPickup.h"
#include "Actors/TimeObjects/PastItemPickup.h"
#include "Actors/TimeObjects/FutureItemPickup.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystemInterface.h"
#include "ActorComponents/InventoryComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Logging/LogMacros.h"


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

void UItemBase::OnDropped(AActor* Instigator, FGuid ItemInstanceID, FVector DropLocation)
{
    // Instance effect/ability removal handled in InventoryComponent before this is called!
    if (!Instigator) return;
    UWorld* World = Instigator->GetWorld();
    if (!World) return;

    FVector SpawnLocation = bEnablePhysicsOnDrop ? Instigator->GetActorLocation() : DropLocation;
    FRotator SpawnRotation = PickupMeshRotation;
    FTransform SpawnTransform = FTransform(SpawnRotation, SpawnLocation);

    AItemPickup* Pickup = World->SpawnActorDeferred<AItemPickup>(AItemPickup::StaticClass(), SpawnTransform);
    if (Pickup)
    {
        Pickup->ItemData = this;
        Pickup->ItemInstanceID = ItemInstanceID;
        UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);
        if (bEnablePhysicsOnDrop && Pickup->OverrideMeshComp)
        {
            Pickup->SetActorLocation(Instigator->GetActorLocation());
            Pickup->OverrideMeshComp->SetSimulatePhysics(true);
            FVector ForwardVector = Instigator->GetActorForwardVector();
            FVector Impulse = ForwardVector * DropImpulseStrength;
            Pickup->OverrideMeshComp->AddImpulse(Impulse, NAME_None, true);
        }
    }
}

void UItemBase::OnDroppedWithTeam(AActor* Instigator, FGuid ItemInstanceID, FGameplayTag TeamTag, FVector DropLocation)
{
    // Instance effect/ability removal handled in InventoryComponent before this is called!
    if (!Instigator) return;
    UWorld* World = Instigator->GetWorld();
    if (!World) return;

    FVector SpawnLocation = bEnablePhysicsOnDrop ? Instigator->GetActorLocation() : DropLocation;
    FRotator SpawnRotation = PickupMeshRotation;
    SpawnLocation.Z = FMath::Max(SpawnLocation.Z, 0.0f);
    FTransform SpawnTransform = FTransform(SpawnRotation, SpawnLocation);

    static FGameplayTag PastTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Past"));
    static FGameplayTag FutureTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Future"));

    if (TeamTag == PastTag)
    {
        APastItemPickup* Pickup = World->SpawnActorDeferred<APastItemPickup>(APastItemPickup::StaticClass(), SpawnTransform);
        if (Pickup)
        {
            Pickup->ItemData = this;
            Pickup->ItemInstanceID = ItemInstanceID;
            UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);
            if (bEnablePhysicsOnDrop && Pickup->OverrideMeshComp)
            {
                Pickup->SetActorLocation(Instigator->GetActorLocation());
                Pickup->OverrideMeshComp->SetSimulatePhysics(true);
                FVector Impulse = Instigator->GetActorForwardVector() * DropImpulseStrength;
                Pickup->OverrideMeshComp->AddImpulse(Impulse, NAME_None, true);
            }
        }
    }
    else if (TeamTag == FutureTag)
    {
        AFutureItemPickup* Pickup = World->SpawnActorDeferred<AFutureItemPickup>(AFutureItemPickup::StaticClass(), SpawnTransform);
        if (Pickup)
        {
            Pickup->ItemData = this;
            Pickup->ItemInstanceID = ItemInstanceID;
            UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);
            if (bEnablePhysicsOnDrop && Pickup->OverrideMeshComp)
            {
                Pickup->SetActorLocation(Instigator->GetActorLocation());
                Pickup->OverrideMeshComp->SetSimulatePhysics(true);
                FVector Impulse = Instigator->GetActorForwardVector() * DropImpulseStrength;
                Pickup->OverrideMeshComp->AddImpulse(Impulse, NAME_None, true);
            }
        }
    }
    else
    {
        AItemPickup* Pickup = World->SpawnActorDeferred<AItemPickup>(AItemPickup::StaticClass(), SpawnTransform);
        if (Pickup)
        {
            Pickup->ItemData = this;
            Pickup->ItemInstanceID = ItemInstanceID;
            UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);
            if (bEnablePhysicsOnDrop && Pickup->OverrideMeshComp)
            {
                Pickup->SetActorLocation(Instigator->GetActorLocation());
                Pickup->OverrideMeshComp->SetSimulatePhysics(true);
                FVector Impulse = Instigator->GetActorForwardVector() * DropImpulseStrength;
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