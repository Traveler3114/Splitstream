#include "ItemBase.h"
#include "GameplayTagContainer.h"
#include "Actors/ItemPickup.h"
#include "Actors/TimeObjects/PastItemPickup.h"
#include "Actors/TimeObjects/FutureItemPickup.h"
#include "Kismet/GameplayStatics.h"

void UItemBase::OnEquipped_Implementation(AActor* Instigator) {}
void UItemBase::OnUsed_Implementation(AActor* Instigator) {}

void UItemBase::OnDropped_Implementation(AActor* Instigator)
{
    if (!Instigator) return;

    UWorld* World = Instigator->GetWorld();
    if (!World) return;

    ItemInstanceID = FGuid::NewGuid();

    FVector Start = Instigator->GetActorLocation() + Instigator->GetActorForwardVector() * 100.0f;
    FVector End = Start - FVector(0, 0, 1000.0f); // Trace 1000 units down

    DrawDebugLine(
        World,
        Start,
        End,
        FColor::Green,
        false,      // persistent lines
        2.0f,       // lifetime in seconds
        0,
        2.0f        // thickness
    );

    FHitResult HitResult;
    FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(ItemDrop), true, Instigator);

    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility,
        TraceParams
    );

    FVector SpawnLocation = bHit ? HitResult.ImpactPoint : Start;
    FRotator SpawnRotation = FRotator::ZeroRotator;
    FTransform SpawnTransform = FTransform(SpawnRotation, SpawnLocation);

    AItemPickup* Pickup = World->SpawnActorDeferred<AItemPickup>(AItemPickup::StaticClass(), SpawnTransform);
    if (Pickup)
    {
        Pickup->ItemData = this;
        UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);
    }
}

void UItemBase::OnDroppedWithTeam_Implementation(AActor* Instigator, FGameplayTag TeamTag)
{
    if (!Instigator) return;
    UWorld* World = Instigator->GetWorld();
    if (!World) return;

    ItemInstanceID = FGuid::NewGuid();

    FVector Start = Instigator->GetActorLocation() + Instigator->GetActorForwardVector() * 100.0f;
    FVector End = Start - FVector(0, 0, 1000.0f); // Trace 1000 units down

    DrawDebugLine(
        World,
        Start,
        End,
        FColor::Green,
        false,      // persistent lines
        2.0f,       // lifetime in seconds
        0,
        2.0f        // thickness
    );

    FHitResult HitResult;
    FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(ItemDrop), true, Instigator);

    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility,
        TraceParams
    );

    FVector SpawnLocation = bHit ? HitResult.ImpactPoint : Start;
    FRotator SpawnRotation = FRotator::ZeroRotator;
    FTransform SpawnTransform = FTransform(SpawnRotation, SpawnLocation);

    static FGameplayTag PastTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Past"));
    static FGameplayTag FutureTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Future"));

    if (TeamTag == PastTag)
    {
        APastItemPickup* Pickup = World->SpawnActorDeferred<APastItemPickup>(APastItemPickup::StaticClass(), SpawnTransform);
        if (Pickup)
        {
            Pickup->ItemData = this;
            UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);
        }
    }
    else if (TeamTag == FutureTag)
    {
        AFutureItemPickup* Pickup = World->SpawnActorDeferred<AFutureItemPickup>(AFutureItemPickup::StaticClass(), SpawnTransform);
        if (Pickup)
        {
            Pickup->ItemData = this;
            UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);
        }
    }
    else
    {
        AItemPickup* Pickup = World->SpawnActorDeferred<AItemPickup>(AItemPickup::StaticClass(), SpawnTransform);
        if (Pickup)
        {
            Pickup->ItemData = this;
            UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);
        }
    }
}