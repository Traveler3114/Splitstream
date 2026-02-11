#include "InteractionComponent.h"
#include "GameFramework/Pawn.h"
#include "ActorComponents/InventoryComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
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


// In InteractionComponent.cpp

void UInteractionComponent::DropEquippedItem(UInventoryComponent* Inventory)
{
    if (!Inventory) return;

    // --- Tunable Parameters ---
    const float ForwardDistance = InteractDistance; // How far in front of character
    const float UpwardOffset = 40.f;    // For the start of the down trace (above "head")
    const float DownwardTrace = 200.f;  // How far to trace down to find a surface
    const float SpawnAboveSurface = 1.f; // Final nudge above ground

    // --- Capsule-based head height (no magic numbers) ---
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character) return;
    UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
    if (!Capsule) return;

    FVector CharLoc = Character->GetActorLocation();
    FVector CharFwd = Character->GetActorForwardVector();

    // Capsule origin is at feet; add capsule half-height for top (head)
    float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
    FVector HeadLoc = CharLoc + FVector(0, 0, CapsuleHalfHeight);

    // Move forward from head position
    FVector StartPos = HeadLoc + CharFwd * ForwardDistance;

    // --- Line trace DOWN to find floor ---
    FVector TraceStart = StartPos + FVector(0, 0, UpwardOffset);
    FVector TraceEnd = StartPos - FVector(0, 0, DownwardTrace);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Character);

    FVector FinalDropLocation = StartPos;
    UWorld* World = GetWorld();
    if (World && World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
    {
        FinalDropLocation = Hit.Location + Hit.Normal * SpawnAboveSurface;
    }
    else
    {
        FinalDropLocation = StartPos + FVector(0, 0, SpawnAboveSurface);
    }

    Inventory->ServerDropActiveItem(FinalDropLocation);
}