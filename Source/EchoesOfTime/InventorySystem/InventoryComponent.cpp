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

    if (GetOwner()->HasAuthority() && DefaultItemClass)
    {
        AddItem(DefaultItemClass, FGuid::NewGuid());
    }
}

void UInventoryComponent::SetActiveSlot(int32 Index)
{
    if (Index >= 0 && Index < Slots.Num())
    {
        ActiveSlotIndex = Index;

        // DEBUG: Show the selected item on screen
        if (GEngine && GetOwner() && GetOwner()->GetWorld())
        {
            FString ItemName = TEXT("Empty Slot");
            if (Slots[Index].ItemClass)
            {
                ItemName = Slots[Index].ItemClass->GetName();
            }
            GEngine->AddOnScreenDebugMessage(
                -1, // Key (lets multiple messages stack)
                2.0f, // Duration in seconds
                FColor::Yellow,
                FString::Printf(TEXT("Selected Slot: %d | Item: %s"), Index + 1, *ItemName)
            );
        }
    }
}

bool UInventoryComponent::AddItem(TSubclassOf<UItemBase> ItemClass, FGuid InstanceID)
{
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (!Slots[i].ItemClass)
        {
            Slots[i].ItemClass = ItemClass;
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
        Slots[Index].ItemClass = nullptr;
        Slots[Index].ItemInstanceID.Invalidate();
        OnInventoryChanged.Broadcast(Slots);
    }
}

UItemBase* UInventoryComponent::CreateItemInstance(const FInventorySlot& Slot) const
{
    if (Slot.ItemClass)
    {
        UItemBase* Item = NewObject<UItemBase>(GetOwner(), Slot.ItemClass);
        if (Item)
        {
            Item->ItemInstanceID = Slot.ItemInstanceID;
        }
        return Item;
    }
    return nullptr;
}

UItemBase* UInventoryComponent::GetActiveItem() const
{
    if (Slots.IsValidIndex(ActiveSlotIndex))
    {
        return CreateItemInstance(Slots[ActiveSlotIndex]);
    }
    return nullptr;
}

void UInventoryComponent::DropActiveItem()
{
    UItemBase* ActiveItem = GetActiveItem();
    if (!ActiveItem) return;

    FGameplayTag TeamTag = GetTeamTag();
    ActiveItem->OnDroppedWithTeam(GetOwner(), TeamTag);

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

void UInventoryComponent::ServerDropActiveItem_Implementation()
{
    DropActiveItem();
}

void UInventoryComponent::ServerAddItem_Implementation(TSubclassOf<UItemBase> ItemClass)
{
    AddItem(ItemClass, FGuid::NewGuid());
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