#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "ItemBase.h"
#include "Engine/Engine.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "DefaultPlayerState.h"
#include "GameplayTagContainer.h"
#include "Actors/TimeObjects/FutureItemPickup.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

FGameplayTag UInventoryComponent::GetTeamTag() const
{
    APawn* Pawn = Cast<APawn>(GetOwner());
    if (!Pawn) return FGameplayTag();
    APlayerState* PS = Pawn->GetPlayerState();
    if (!PS) return FGameplayTag();
    UAbilitySystemComponent* ASC = nullptr;
    if (IAbilitySystemInterface* IFace = Cast<IAbilitySystemInterface>(PS))
        ASC = IFace->GetAbilitySystemComponent();
    if (!ASC) return FGameplayTag();

    static FGameplayTag PastTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Past"));
    static FGameplayTag FutureTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Future"));
    if (ASC->HasMatchingGameplayTag(PastTag))
        return PastTag;
    if (ASC->HasMatchingGameplayTag(FutureTag))
        return FutureTag;
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
    if (Index >= 0 && Index < Slots.Num())
    {
        ActiveSlotIndex = Index;

        if (GEngine && GetOwner() && GetOwner()->GetWorld())
        {
            FString ItemName = TEXT("Empty Slot");
            if (Slots[Index].ItemAsset)
            {
                ItemName = Slots[Index].ItemAsset->GetName();
            }
            GEngine->AddOnScreenDebugMessage(
                -1,
                2.0f,
                FColor::Yellow,
                FString::Printf(TEXT("Selected Slot: %d | Item: %s"), Index + 1, *ItemName)
            );
        }
    }
}

bool UInventoryComponent::AddItem(UItemBase* ItemAsset, FGuid InstanceID)
{
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (!Slots[i].ItemAsset)
        {
            Slots[i].ItemAsset = ItemAsset;
            Slots[i].ItemInstanceID = InstanceID;
            OnInventoryChanged.Broadcast(Slots);
            return true;
        }
    }
    return false;
}

void UInventoryComponent::RemoveItem(int32 Index)
{
    if (Slots.IsValidIndex(Index))
    {
        Slots[Index].ItemAsset = nullptr;
        Slots[Index].ItemInstanceID.Invalidate();
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
    ActiveSlot.ItemAsset->OnDroppedWithTeam(GetOwner(), ActiveSlot.ItemInstanceID, TeamTag, DropLocation);
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
    // Optionally notify UI or other systems
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