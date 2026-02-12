#include "InteractionComponent.h"
#include "GameFramework/Pawn.h"
#include "ActorComponents/InventoryComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Actors/ItemPickup.h"
#include "Interfaces/IInteractable.h"

UInteractionComponent::UInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // Let your pawn tick and call UpdateInteractHighlight when needed.
    InteractHighlightTimer = 0.f;
}



void UInteractionComponent::StartDropPreview()
{
    if (DropPreviewMesh) return; // Already active
    // Get locally equipped item
    APawn* PawnOwner = Cast<APawn>(GetOwner());
    UInventoryComponent* Inventory = PawnOwner ? PawnOwner->FindComponentByClass<UInventoryComponent>() : nullptr;
    if (!Inventory) return;

    FInventorySlot ActiveSlot = Inventory->GetActiveItem();
    UItemBase* ItemAsset = ActiveSlot.ItemAsset;
    if (!ItemAsset || !ItemAsset->ItemPickupToSpawn) return;

    DropPreviewMesh = NewObject<UStaticMeshComponent>(GetOwner());
    DropPreviewMesh->RegisterComponent();
    DropPreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DropPreviewMesh->SetMobility(EComponentMobility::Movable);
    DropPreviewMesh->SetMaterial(0, DropGhostMaterial);
    DropPreviewMesh->SetStaticMesh(GetGhostMeshFromPickup(ItemAsset));
    DropPreviewMesh->SetVisibility(false);
    DropPreviewMesh->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
}

void UInteractionComponent::TickDropPreview(FVector CamLoc, FRotator CamRot)
{
    if (!DropPreviewMesh) return;

    FVector Start = CamLoc;
    FVector Forward = CamRot.Vector();
    FVector End = Start + Forward * DropDistance;
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        float DotUp = FVector::DotProduct(Hit.Normal, FVector::UpVector);
        if (DotUp > 0.5f)
        {
            DropPreviewMesh->SetVisibility(true);

            // 1. Set rotation to align flat with floor
            FVector GhostForward = Forward - Hit.Normal * FVector::DotProduct(Forward, Hit.Normal);
            if (!GhostForward.Normalize()) GhostForward = FVector::ForwardVector;
            FMatrix FloorAligned(
                GhostForward,
                FVector::CrossProduct(Hit.Normal, GhostForward),
                Hit.Normal,
                FVector::ZeroVector
            );
            FRotator Rot = FloorAligned.Rotator();
            DropPreviewMesh->SetWorldRotation(Rot);
            DropPreviewMesh->AddLocalRotation(FRotator(-90.f, 0.f, 0.f));

            // 2. Get socket offset in local/component space AFTER rotation
            static const FName GroundSocketName(TEXT("GroundAlign"));
            FVector LocalSocketOffset = FVector::ZeroVector;
            if (DropPreviewMesh->DoesSocketExist(GroundSocketName))
            {
                LocalSocketOffset = DropPreviewMesh->GetSocketLocation(GroundSocketName) - DropPreviewMesh->GetComponentLocation();
            }

            // 3. Place mesh so socket sits at Hit.Location
            FVector MeshWorldLocation = Hit.Location - LocalSocketOffset;
            DropPreviewMesh->SetWorldLocation(MeshWorldLocation);

            // 4. Update transform
            CachedDropTransform = FTransform(
                DropPreviewMesh->GetComponentRotation(),
                DropPreviewMesh->GetComponentLocation(),
                DropPreviewMesh->GetRelativeScale3D()
            );

            bDropPreviewIsValid = true;
            return;
        }
    }
    DropPreviewMesh->SetVisibility(false);
    bDropPreviewIsValid = false;
}

void UInteractionComponent::StopDropPreview()
{
    if (DropPreviewMesh)
    {
        DropPreviewMesh->DestroyComponent();
        DropPreviewMesh = nullptr;
        bDropPreviewIsValid = false;
    }
}

void UInteractionComponent::OnReleaseDropPreview(UInventoryComponent* Inventory)
{
    if (bDropPreviewIsValid && Inventory)
    {
        Inventory->ServerDropActiveItem(CachedDropTransform);
    }
    StopDropPreview();
}

UStaticMesh* UInteractionComponent::GetGhostMeshFromPickup(UItemBase* ItemAsset)
{
    if (!ItemAsset) return nullptr;
    if (const AItemPickup* PickupCDO = Cast<AItemPickup>(ItemAsset->ItemPickupToSpawn->GetDefaultObject()))
        return PickupCDO->OverrideMeshComp ? PickupCDO->OverrideMeshComp->GetStaticMesh() : nullptr;
    return nullptr;
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
