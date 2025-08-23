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

namespace
{
    // Helper to determine if a surface is "floor-like" (not a wall or ceiling)
    bool IsSurfaceFloorLike(const FVector& Normal, float MinUpDot = 0.7f)
    {
        // Dot with up vector (Z+). 1.0 = perfectly up, 0 = perpendicular, -1 = down.
        return FVector::DotProduct(Normal, FVector::UpVector) >= MinUpDot;
    }
}

void UItemBase::OnEquipped_Implementation(AActor* Instigator) {}
void UItemBase::OnUsed_Implementation(AActor* Instigator) {}

void UItemBase::OnDropped_Implementation(AActor* Instigator)
{
    if (!Instigator) return;

    UWorld* World = Instigator->GetWorld();
    if (!World) return;

    ItemInstanceID = FGuid::NewGuid();

    FVector CameraLoc;
    FRotator CameraRot;

    // Get camera location and rotation
    if (ACharacter* Character = Cast<ACharacter>(Instigator))
    {
        if (AController* Controller = Character->GetController())
        {
            Controller->GetPlayerViewPoint(CameraLoc, CameraRot);
        }
        else if (UCameraComponent* Camera = Character->FindComponentByClass<UCameraComponent>())
        {
            CameraLoc = Camera->GetComponentLocation();
            CameraRot = Camera->GetComponentRotation();
        }
        else
        {
            CameraLoc = Instigator->GetActorLocation();
            CameraRot = Instigator->GetActorRotation();
        }
    }
    else
    {
        CameraLoc = Instigator->GetActorLocation();
        CameraRot = Instigator->GetActorRotation();
    }

    // Configurable trace distance
    float TraceDistance = 1000.0f;
    FVector TraceEnd = CameraLoc + CameraRot.Vector() * TraceDistance;

    // First trace: from camera forward
    FHitResult ForwardHit;
    FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(ItemDrop), true, Instigator);

    DrawDebugLine(World, CameraLoc, TraceEnd, FColor::Green, false, 2.0f, 0, 2.0f);

    bool bForwardHit = World->LineTraceSingleByChannel(
        ForwardHit,
        CameraLoc,
        TraceEnd,
        ECC_Visibility,
        TraceParams
    );

    FVector SpawnLocation = CameraLoc;
    bool bValidDrop = false;

    if (bForwardHit)
    {
        // Check if the surface is floor-like
        if (IsSurfaceFloorLike(ForwardHit.ImpactNormal))
        {
            SpawnLocation = ForwardHit.ImpactPoint;
            bValidDrop = true;
        }
        else
        {
            // If not, do a downward trace from the hit point
            FVector DownTraceStart = ForwardHit.ImpactPoint + FVector(0, 0, 10.0f); // Slightly above
            FVector DownTraceEnd = DownTraceStart - FVector(0, 0, 2000.0f);

            DrawDebugLine(World, DownTraceStart, DownTraceEnd, FColor::Blue, false, 2.0f, 0, 2.0f);

            FHitResult DownHit;
            if (World->LineTraceSingleByChannel(
                DownHit,
                DownTraceStart,
                DownTraceEnd,
                ECC_Visibility,
                TraceParams
            ))
            {
                if (IsSurfaceFloorLike(DownHit.ImpactNormal))
                {
                    SpawnLocation = DownHit.ImpactPoint;
                    bValidDrop = true;
                }
            }
        }
    }

    if (!bValidDrop)
    {
        // Fallback: drop in front of player at camera height
        SpawnLocation = CameraLoc + CameraRot.Vector() * 100.0f;
    }

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

    FVector CameraLoc;
    FRotator CameraRot;

    // Get camera location and rotation
    if (ACharacter* Character = Cast<ACharacter>(Instigator))
    {
        if (AController* Controller = Character->GetController())
        {
            Controller->GetPlayerViewPoint(CameraLoc, CameraRot);
        }
        else if (UCameraComponent* Camera = Character->FindComponentByClass<UCameraComponent>())
        {
            CameraLoc = Camera->GetComponentLocation();
            CameraRot = Camera->GetComponentRotation();
        }
        else
        {
            CameraLoc = Instigator->GetActorLocation();
            CameraRot = Instigator->GetActorRotation();
        }
    }
    else
    {
        CameraLoc = Instigator->GetActorLocation();
        CameraRot = Instigator->GetActorRotation();
    }

    float TraceDistance = 1000.0f;
    FVector TraceEnd = CameraLoc + CameraRot.Vector() * TraceDistance;

    FHitResult ForwardHit;
    FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(ItemDrop), true, Instigator);

    DrawDebugLine(World, CameraLoc, TraceEnd, FColor::Green, false, 2.0f, 0, 2.0f);

    bool bForwardHit = World->LineTraceSingleByChannel(
        ForwardHit,
        CameraLoc,
        TraceEnd,
        ECC_Visibility,
        TraceParams
    );

    FVector SpawnLocation = CameraLoc;
    bool bValidDrop = false;

    if (bForwardHit)
    {
        if (IsSurfaceFloorLike(ForwardHit.ImpactNormal))
        {
            SpawnLocation = ForwardHit.ImpactPoint;
            bValidDrop = true;
        }
        else
        {
            FVector DownTraceStart = ForwardHit.ImpactPoint + FVector(0, 0, 10.0f);
            FVector DownTraceEnd = DownTraceStart - FVector(0, 0, 2000.0f);

            DrawDebugLine(World, DownTraceStart, DownTraceEnd, FColor::Blue, false, 2.0f, 0, 2.0f);

            FHitResult DownHit;
            if (World->LineTraceSingleByChannel(
                DownHit,
                DownTraceStart,
                DownTraceEnd,
                ECC_Visibility,
                TraceParams
            ))
            {
                if (IsSurfaceFloorLike(DownHit.ImpactNormal))
                {
                    SpawnLocation = DownHit.ImpactPoint;
                    bValidDrop = true;
                }
            }
        }
    }

    if (!bValidDrop)
    {
        SpawnLocation = CameraLoc + CameraRot.Vector() * 100.0f;
    }

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