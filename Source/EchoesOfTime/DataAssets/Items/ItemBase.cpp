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

    // Spawn at character location if physics is enabled, otherwise DropLocation
    FVector SpawnLocation = bEnablePhysicsOnDrop ? Instigator->GetActorLocation() : DropLocation;
    FRotator SpawnRotation = PickupMeshRotation;
    FTransform SpawnTransform = FTransform(SpawnRotation, SpawnLocation);

    AItemPickup* Pickup = World->SpawnActorDeferred<AItemPickup>(AItemPickup::StaticClass(), SpawnTransform);
    if (Pickup)
    {
        Pickup->ItemData = this;
        Pickup->ItemInstanceID = ItemInstanceID;
        UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);

        // Physics and impulse if enabled
        if (bEnablePhysicsOnDrop && Pickup->OverrideMeshComp)
        {
            // Just to ensure correct position
            Pickup->SetActorLocation(Instigator->GetActorLocation());

            // Enable physics
            Pickup->OverrideMeshComp->SetSimulatePhysics(true);

            // Add impulse in character's forward vector
            FVector ForwardVector = Instigator->GetActorForwardVector();
            FVector Impulse = ForwardVector * DropImpulseStrength;
            Pickup->OverrideMeshComp->AddImpulse(Impulse, NAME_None, true);
        }
    }
}

void UItemBase::OnDroppedWithTeam(AActor* Instigator, FGuid ItemInstanceID, FGameplayTag TeamTag, FVector DropLocation)
{
    RemoveGrantedGameplayEffects(Instigator);

    if (!Instigator) return;
    UWorld* World = Instigator->GetWorld();
    if (!World) return;

    // Spawn at character location if physics is enabled, otherwise DropLocation
    FVector SpawnLocation = bEnablePhysicsOnDrop ? Instigator->GetActorLocation() : DropLocation;
    FRotator SpawnRotation = PickupMeshRotation;
    SpawnLocation.Z = FMath::Max(SpawnLocation.Z, 0.0f); // Slightly raise the drop location
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