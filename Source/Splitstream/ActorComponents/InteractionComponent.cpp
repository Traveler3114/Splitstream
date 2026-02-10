#include "InteractionComponent.h"
#include "GameFramework/Pawn.h"
#include "ActorComponents/InventoryComponent.h"
#include "Camera/CameraComponent.h"
#include "Interfaces/IInteractable.h"

UInteractionComponent::UInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // Let your pawn tick and call UpdateInteractHighlight when needed.
    InteractHighlightTimer = 0.f;
}

bool UInteractionComponent::GetForwardTraceResult(
    FVector Start, FRotator Rotation, float TraceDistance,
    FHitResult& OutHit, FVector& OutTraceEnd) const
{
    FVector End = Start + Rotation.Vector() * TraceDistance;
    OutTraceEnd = End;
    FCollisionQueryParams Params;
    if (GetOwner()) Params.AddIgnoredActor(GetOwner());
    UWorld* World = GetWorld();
    if (!World) return false;
    return World->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);
}

void UInteractionComponent::UpdateInteractHighlight(FVector Start, FRotator Rotation)
{
    FHitResult Hit;
    FVector TraceEnd;
    bool bHit = GetForwardTraceResult(Start, Rotation, InteractDistance, Hit, TraceEnd);

    AActor* HitActor = bHit ? Hit.GetActor() : nullptr;

    if (HighlightedActor && HighlightedActor != HitActor)
    {
        if (HighlightedActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
            IInteractable::Execute_SetHighlighted(HighlightedActor, false);
        HighlightedActor = nullptr;
    }
    if (HitActor && HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
    {
        if (HitActor != HighlightedActor)
        {
            IInteractable::Execute_SetHighlighted(HitActor, true);
            HighlightedActor = HitActor;
        }
    }
}

void UInteractionComponent::HandleHoldInteractStart(
    AActor* Instigator, FVector Start, FRotator Rotation)
{
    if (bIsHoldingInteract) return;
    bIsHoldingInteract = true;

    FHitResult Hit; FVector TraceEnd;
    if (!GetForwardTraceResult(Start, Rotation, InteractDistance, Hit, TraceEnd)) return;
    AActor* HitActor = Hit.GetActor();
    if (!HitActor) return;

    ProgressiveActor = HitActor;

    if (HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()) &&
        IInteractable::Execute_IsProgressiveInteract(HitActor))
    {
        IInteractable::Execute_Interact(HitActor, Instigator);
    }
}

void UInteractionComponent::HandleHoldInteractStop(AActor* Instigator)
{
    if (!bIsHoldingInteract) return;
    bIsHoldingInteract = false;

    if (ProgressiveActor.IsValid())
    {
        AActor* ActorPtr = ProgressiveActor.Get();
        if (ActorPtr && ActorPtr->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
        {
            IInteractable::Execute_CancelInteract(ActorPtr, Instigator);
        }
        ProgressiveActor = nullptr;
    }
}

void UInteractionComponent::HandleInstantInteract(
    AActor* Instigator, FVector Start, FRotator Rotation,
    TFunction<void(AActor*)> ServerInteractCallback)
{
    FHitResult Hit; FVector TraceEnd;
    if (!GetForwardTraceResult(Start, Rotation, InteractDistance, Hit, TraceEnd)) return;
    AActor* HitActor = Hit.GetActor();
    if (!HitActor || !HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass())) return;

    if (!IInteractable::Execute_IsProgressiveInteract(HitActor))
    {
        IInteractable::Execute_Interact(HitActor, Instigator);
    }

    // The pawn should always call its ServerHandleInteract from here (the callback).
    if (APawn* PawnOwner = Cast<APawn>(GetOwner()); PawnOwner && PawnOwner->IsLocallyControlled() && PawnOwner->HasAuthority()) return;
    if (ServerInteractCallback)
    {
        ServerInteractCallback(HitActor);
    }
}


void UInteractionComponent::DropEquippedItem(UInventoryComponent* Inventory, UCameraComponent* Camera, float TraceDistance)
{
    if (!Inventory || !Camera) return;
    float DropDist = TraceDistance > 0.f ? TraceDistance : InteractDistance;

    FVector Start = Camera->GetComponentLocation();
    FRotator Rot = Camera->GetComponentRotation();
    FVector End = Start + Rot.Vector() * DropDist;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    FVector DropLocation = End;
    UWorld* World = GetWorld();
    if (World)
    {
        if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
        {
            DropLocation = Hit.bBlockingHit ? Hit.ImpactPoint : End;
        }

        // Optionally, trace down to ground
        FHitResult DownHit;
        FVector DownStart = DropLocation + FVector(0,0,50);
        FVector DownEnd = DropLocation - FVector(0,0,200);
        if (World->LineTraceSingleByChannel(DownHit, DownStart, DownEnd, ECC_Visibility, Params))
        {
            DropLocation = DownHit.Location;
        }
    }
    Inventory->ServerDropActiveItem(DropLocation);
}