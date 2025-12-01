#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "DefaultPlayerState.h"
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

    FString TeamName = PS->GetTeamName();
    if (TeamName == "Past")
    {
        return FGameplayTag::RequestGameplayTag(TEXT("Team.Past"));
    }
    else if (TeamName == "Future")
    {
        return FGameplayTag::RequestGameplayTag(TEXT("Team.Future"));
    }

    return FGameplayTag(); // None
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    Slots.SetNum(SlotCount);

    if (GetOwner()->HasAuthority() && DefaultItemAsset)
    {
        AddItem(DefaultItemAsset, FGuid::NewGuid());
    }
}

void UInventoryComponent::SetActiveSlot(int32 Index)
{
    UE_LOG(LogTemp, Warning, TEXT("SetActiveSlot called on %s with index %d. Owner: %s"),
        GetOwner() && GetOwner()->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"),
        Index,
        *GetNameSafe(GetOwner())
    );

    if (GetOwner()->HasAuthority())
    {
        // Unequip previous item, if any and if it's different from new
        if (ActiveSlotIndex != Index && Slots.IsValidIndex(ActiveSlotIndex))
        {
            FInventorySlot& OldSlot = Slots[ActiveSlotIndex];
            if (OldSlot.ItemAsset)
            {
                UE_LOG(LogTemp, Warning, TEXT("Calling OnUnequipped for slot %d on %s"), ActiveSlotIndex,
                    GetOwner() && GetOwner()->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"));
                OldSlot.ItemAsset->OnUnequipped(GetOwner());
            }
        }

        ActiveSlotIndex = Index;
        FInventorySlot& NewSlot = Slots[Index];
        if (NewSlot.ItemAsset)
        {
            UE_LOG(LogTemp, Warning, TEXT("Calling OnEquipped for slot %d on %s"), Index,
                GetOwner() && GetOwner()->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"));
            NewSlot.ItemAsset->OnEquipped(GetOwner());
        }
        OnRep_ActiveSlotIndex();
    }
}

bool UInventoryComponent::AddItem(UItemBase* ItemAsset, FGuid InstanceID)
{
    UE_LOG(LogTemp, Warning, TEXT("AddItem called on %s. Item: %s, Owner: %s"),
        GetOwner() && GetOwner()->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"),
        *GetNameSafe(ItemAsset),
        *GetNameSafe(GetOwner())
    );

    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (!Slots[i].ItemAsset)
        {
            Slots[i].ItemAsset = ItemAsset;
            Slots[i].ItemInstanceID = InstanceID;
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
    UE_LOG(LogTemp, Warning, TEXT("RemoveItem called on %s. Index: %d, Owner: %s"),
        GetOwner() && GetOwner()->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"),
        Index,
        *GetNameSafe(GetOwner())
    );

    if (Slots.IsValidIndex(Index))
    {
        if (Slots[Index].ItemAsset)
        {
            UE_LOG(LogTemp, Warning, TEXT("Calling OnUnequipped for removed slot %d on %s"), Index,
                GetOwner() && GetOwner()->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"));
            Slots[Index].ItemAsset->OnUnequipped(GetOwner());
        }

        // Shift all items after Index down by one
        for (int32 i = Index; i < Slots.Num() - 1; ++i)
        {
            Slots[i] = Slots[i + 1];
        }
        // Clear last slot
        Slots[Slots.Num() - 1].ItemAsset = nullptr;
        Slots[Slots.Num() - 1].ItemInstanceID.Invalidate();

        // Fix active slot index if needed
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
    //ActiveSlot.ItemAsset->OnDroppedWithTeam(GetOwner(), ActiveSlot.ItemInstanceID, TeamTag, DropLocation);
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