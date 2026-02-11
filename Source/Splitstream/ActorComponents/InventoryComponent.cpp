#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/StaticMeshComponent.h"
#include "DefaultPlayerState.h"
#include "Characters/CivilianCharacter.h"
#include "GameplayTagContainer.h"
#include "Actors/TimeObjects/FutureItemPickup.h"

UInventoryComponent::UInventoryComponent()
{
    SetIsReplicatedByDefault(true);
    PrimaryComponentTick.bCanEverTick = false;
}

FGameplayTag UInventoryComponent::GetTeamTag() const
{
    APawn* Pawn = Cast<APawn>(GetOwner());
    if (!Pawn) return FGameplayTag();

    ADefaultPlayerState* PS = Cast<ADefaultPlayerState>(Pawn->GetPlayerState());
    if (!PS) return FGameplayTag();

    // Directly return the canonical tag, no string conversion!
    return PS->GetTeamTag();
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    Slots.SetNum(SlotCount);

    if (GetOwner()->HasAuthority())
    {
        for (UItemBase* Item : DefaultItemAssets)
        {
            if (Item)
            {
                AddItem(Item, FGuid::NewGuid());
            }
        }
    }
    OnInventoryChanged.RemoveDynamic(this, &UInventoryComponent::UpdateEquippedItemMesh);
    OnInventoryChanged.AddDynamic(this, &UInventoryComponent::UpdateEquippedItemMesh);
}

void UInventoryComponent::SetActiveSlot(int32 Index)
{
    if (Index == ActiveSlotIndex)
        return;

    // Unequip previous
    if (Slots.IsValidIndex(ActiveSlotIndex))
    {
        FInventorySlot& OldSlot = Slots[ActiveSlotIndex];
        if (OldSlot.ItemAsset)
        {
            if (!OldSlot.ItemAsset->bApplyGameplayEffectsPassively)
            {
                OldSlot.ItemAsset->RemoveGrantedEffectsFrom(GetOwner(), OldSlot.GrantedGameplayEffectHandles);
                OldSlot.GrantedGameplayEffectHandles.Empty();
            }
            if (!OldSlot.ItemAsset->bApplyGameplayAbilitiesPassively)
            {
                OldSlot.ItemAsset->RemoveGrantedAbilitiesFrom(GetOwner(), OldSlot.GrantedAbilityHandles);
                OldSlot.GrantedAbilityHandles.Empty();
            }
            OldSlot.ItemAsset->OnUnequipped(GetOwner());
        }
    }

    ActiveSlotIndex = Index;

    // Equip new
    if (Slots.IsValidIndex(ActiveSlotIndex))
    {
        FInventorySlot& NewSlot = Slots[ActiveSlotIndex];
        if (NewSlot.ItemAsset)
        {
            if (!NewSlot.ItemAsset->bApplyGameplayEffectsPassively)
            {
                NewSlot.ItemAsset->GrantEffectsTo(GetOwner(), NewSlot.GrantedGameplayEffectHandles);
            }
            if (!NewSlot.ItemAsset->bApplyGameplayAbilitiesPassively)
            {
                NewSlot.ItemAsset->GrantAbilitiesTo(GetOwner(), NewSlot.GrantedAbilityHandles);
            }
            NewSlot.ItemAsset->OnEquipped(GetOwner());
        }
    }

    OnRep_ActiveSlotIndex();
}


void UInventoryComponent::UpdateEquippedItemMesh(const TArray<FInventorySlot>& InSlots)
{
    if (!EquippedItemMeshComp)
        return;

    FInventorySlot ActiveSlot = GetActiveItem();
    UItemBase* ItemAsset = ActiveSlot.ItemAsset;

    UStaticMesh* NewMesh = nullptr;
    FVector RelativeLocation = FVector::ZeroVector;
    FRotator RelativeRotation = FRotator::ZeroRotator;
    FVector RelativeScale = FVector::OneVector;

    // Try to get mesh and transform from the Pickup class defaults (if set)
    if (ItemAsset && ItemAsset->ItemPickupToSpawn)
    {
        // "CDO" = Class Default Object (the archetype)
        if (const AItemPickup* PickupCDO = Cast<AItemPickup>(ItemAsset->ItemPickupToSpawn->GetDefaultObject()))
        {
            if (PickupCDO->OverrideMeshComp)
            {
                NewMesh = PickupCDO->OverrideMeshComp->GetStaticMesh();
                RelativeLocation = PickupCDO->OverrideMeshComp->GetRelativeLocation();
                RelativeRotation = PickupCDO->OverrideMeshComp->GetRelativeRotation();
                RelativeScale = PickupCDO->OverrideMeshComp->GetRelativeScale3D();
            }
        }
    }
    // Only update the mesh if needed
    if (EquippedItemMeshComp->GetStaticMesh() != NewMesh)
    {
        EquippedItemMeshComp->SetStaticMesh(nullptr);
        EquippedItemMeshComp->SetStaticMesh(NewMesh);
    }

    if (NewMesh)
    {
        EquippedItemMeshComp->SetRelativeLocation(RelativeLocation);
        EquippedItemMeshComp->SetRelativeRotation(RelativeRotation);
        EquippedItemMeshComp->SetRelativeScale3D(RelativeScale);
    }

    // Compute ADS if we have a mesh
    //if (EquippedItemMeshComp->GetStaticMesh())
    //{
    //    static const FName ADSSocket(TEXT("ADS"));
    //    if (EquippedItemMeshComp->DoesSocketExist(ADSSocket))
    //    {
    //        const FTransform SocketTransform =
    //            EquippedItemMeshComp->GetSocketTransform(ADSSocket, ERelativeTransformSpace::RTS_Component);

    //        CameraAimLocation = SocketTransform.GetLocation();
    //        CameraAimRotation = SocketTransform.Rotator();
    //    }
    //}
}


bool UInventoryComponent::AddItem(UItemBase* ItemAsset, FGuid InstanceID)
{
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (!Slots[i].ItemAsset)
        {
            Slots[i].ItemAsset = ItemAsset;
            Slots[i].ItemInstanceID = InstanceID;

            // --- Handle effect/ability application per-instance ---
            if (ItemAsset && ItemAsset->bApplyGameplayEffectsPassively)
            {
                ItemAsset->GrantEffectsTo(GetOwner(), Slots[i].GrantedGameplayEffectHandles);
            }
            if (ItemAsset && ItemAsset->bApplyGameplayAbilitiesPassively)
            {
                ItemAsset->GrantAbilitiesTo(GetOwner(), Slots[i].GrantedAbilityHandles);
            }

            OnInventoryChanged.Broadcast(Slots);

            bool bWasInventoryEmpty = true;
            for (const FInventorySlot& Slot : Slots)
            {
                if (Slot.ItemAsset && &Slot != &Slots[i])
                {
                    bWasInventoryEmpty = false;
                    break;
                }
            }
            if (bWasInventoryEmpty)
            {
                SetActiveSlot(i);
            }
            return true;
        }
    }
    return false;
}

void UInventoryComponent::RemoveItem(int32 Index)
{
    if (Slots.IsValidIndex(Index))
    {
        auto* Item = Slots[Index].ItemAsset;
        // Remove instance-granted effects/abilities before removing from inventory
        if (Item)
        {
            Item->RemoveGrantedEffectsFrom(GetOwner(), Slots[Index].GrantedGameplayEffectHandles);
            Item->RemoveGrantedAbilitiesFrom(GetOwner(), Slots[Index].GrantedAbilityHandles);
            Slots[Index].GrantedGameplayEffectHandles.Empty();
            Slots[Index].GrantedAbilityHandles.Empty();

            Item->OnRemovedFromInventory(GetOwner());
        }

        for (int32 i = Index; i < Slots.Num() - 1; ++i)
        {
            Slots[i] = Slots[i + 1];
        }
        Slots[Slots.Num() - 1].ItemAsset = nullptr;
        Slots[Slots.Num() - 1].ItemInstanceID.Invalidate();
        Slots[Slots.Num() - 1].GrantedGameplayEffectHandles.Empty();
        Slots[Slots.Num() - 1].GrantedAbilityHandles.Empty();

        if (ActiveSlotIndex >= Slots.Num())
        {
            ActiveSlotIndex = Slots.Num() - 1;
        }
        else if (ActiveSlotIndex > Index)
        {
            --ActiveSlotIndex;
        }
        else if (ActiveSlotIndex == Index)
        {
            ActiveSlotIndex = INDEX_NONE;
        }

        OnInventoryChanged.Broadcast(Slots);
    }
}

FInventorySlot UInventoryComponent::CreateSlot(UItemBase* ItemAsset, FGuid InstanceID) const
{
    FInventorySlot Slot;
    Slot.ItemAsset = ItemAsset;
    Slot.ItemInstanceID = InstanceID;
    return Slot;
}

FInventorySlot UInventoryComponent::GetActiveItem() const
{
    if (Slots.IsValidIndex(ActiveSlotIndex))
    {
        return Slots[ActiveSlotIndex];
    }
    return FInventorySlot();
}

void UInventoryComponent::DropActiveItem(FVector DropLocation)
{
    FInventorySlot ActiveSlot = GetActiveItem();
    if (!ActiveSlot.ItemAsset) return;
    FGameplayTag TeamTag = GetTeamTag();

    // Remove granted handles before drop
    int DropIndex = ActiveSlotIndex;
    if (Slots.IsValidIndex(DropIndex))
    {
        Slots[DropIndex].ItemAsset->RemoveGrantedEffectsFrom(GetOwner(), Slots[DropIndex].GrantedGameplayEffectHandles);
        Slots[DropIndex].ItemAsset->RemoveGrantedAbilitiesFrom(GetOwner(), Slots[DropIndex].GrantedAbilityHandles);
        Slots[DropIndex].GrantedGameplayEffectHandles.Empty();
        Slots[DropIndex].GrantedAbilityHandles.Empty();
    }

    ActiveSlot.ItemAsset->OnDropped(GetOwner(), ActiveSlot.ItemInstanceID, DropLocation);
    RemoveItem(ActiveSlotIndex);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UInventoryComponent, Slots);
    DOREPLIFETIME(UInventoryComponent, ActiveSlotIndex);
}

void UInventoryComponent::OnRep_Slots()
{
    OnInventoryChanged.Broadcast(Slots);
}

void UInventoryComponent::OnRep_ActiveSlotIndex()
{
    OnInventoryChanged.Broadcast(Slots);
}

void UInventoryComponent::ServerSetActiveSlot_Implementation(int32 Index)
{
    SetActiveSlot(Index);
}

void UInventoryComponent::ServerDropActiveItem_Implementation(FVector DropLocation)
{
    DropActiveItem(DropLocation);
}

void UInventoryComponent::ServerAddItem_Implementation(UItemBase* ItemAsset)
{
    AddItem(ItemAsset, FGuid::NewGuid());
}

// ---- FUTURE ITEM INVALIDATION ----

void UInventoryComponent::RegisterFutureInstance(FGuid ItemInstanceID)
{
    if (!RegisteredFutureInstances.Contains(ItemInstanceID))
    {
        RegisteredFutureInstances.Add(ItemInstanceID);
        AFutureItemPickup::OnFutureItemInvalidated.AddUObject(this, &UInventoryComponent::HandleFutureItemInvalidated);
    }
}

void UInventoryComponent::RemoveItemByInstanceID(FGuid ItemInstanceID)
{
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (Slots[i].ItemInstanceID == ItemInstanceID)
        {
            RemoveItem(i);
            break;
        }
    }
}

void UInventoryComponent::HandleFutureItemInvalidated(FGuid InvalidID)
{
    RemoveItemByInstanceID(InvalidID);
}

void UInventoryComponent::BeginDestroy()
{
    AFutureItemPickup::OnFutureItemInvalidated.RemoveAll(this);
    Super::BeginDestroy();
}