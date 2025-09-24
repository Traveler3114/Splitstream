#include "ItemBase.h"
#include "GameplayTagContainer.h"
#include "Actors/ItemPickup.h"
#include "Actors/TimeObjects/PastItemPickup.h"
#include "Actors/TimeObjects/FutureItemPickup.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

void UItemBase::OnEquipped(AActor* Instigator)
{
    IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(Instigator);
    if (!AbilityInterface) return;

    UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent();
    if (!ASC) return;

    for (TSubclassOf<UGameplayEffect> EffectClass : GrantedGameplayEffects)
    {
        if (EffectClass)
        {
            FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectToSelf(EffectClass->GetDefaultObject<UGameplayEffect>(), 1.0f, ASC->MakeEffectContext());
            GrantedGameplayEffectHandles.Add(Handle);
        }
    }
}

void UItemBase::OnUnequipped(AActor* Instigator)
{
    RemoveGrantedGameplayEffects(Instigator);
}

void UItemBase::OnDropped(AActor* Instigator, FGuid ItemInstanceID, FVector DropLocation)
{
    RemoveGrantedGameplayEffects(Instigator);

    if (!Instigator) return;
    UWorld* World = Instigator->GetWorld();
    if (!World) return;

    FRotator SpawnRotation = PickupMeshRotation;
    FTransform SpawnTransform = FTransform(SpawnRotation, DropLocation);

    AItemPickup* Pickup = World->SpawnActorDeferred<AItemPickup>(AItemPickup::StaticClass(), SpawnTransform);
    if (Pickup)
    {
        Pickup->ItemData = this;
        Pickup->ItemInstanceID = ItemInstanceID;
        UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);
    }
}

void UItemBase::OnDroppedWithTeam(AActor* Instigator, FGuid ItemInstanceID, FGameplayTag TeamTag, FVector DropLocation)
{
    RemoveGrantedGameplayEffects(Instigator);

    if (!Instigator) return;
    UWorld* World = Instigator->GetWorld();
    if (!World) return;

    FRotator SpawnRotation = PickupMeshRotation;
    DropLocation.Z = FMath::Max(DropLocation.Z, 0.0f); // Slightly raise the drop location to avoid clipping into the ground
    FTransform SpawnTransform = FTransform(SpawnRotation, DropLocation);
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
        }
    }
}

void UItemBase::OnUsed(AActor* Instigator)
{
    // Optional: implement use logic if needed
}

void UItemBase::RemoveGrantedGameplayEffects(AActor* Instigator)
{
    IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(Instigator);
    if (!AbilityInterface) return;

    UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent();
    if (!ASC) return;

    for (const FActiveGameplayEffectHandle& Handle : GrantedGameplayEffectHandles)
    {
        ASC->RemoveActiveGameplayEffect(Handle);
    }
    GrantedGameplayEffectHandles.Empty();
}