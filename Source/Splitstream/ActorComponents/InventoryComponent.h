#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataAssets/ItemBase.h"
#include "GameplayEffectTypes.h"
#include "GameplayAbilitySpec.h"
#include "InventoryComponent.generated.h"

/** Represents a single inventory slot with an item asset, instance ID, and runtime GAS handles. */
USTRUCT(BlueprintType)
struct FInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UItemBase* ItemAsset = nullptr; // Pointer to DataAsset

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid ItemInstanceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class ACivilianCharacter* OwnerCivilian = nullptr;

    // Per-instance effect/ability handles (not properties, runtime only!)
    TArray<FActiveGameplayEffectHandle> GrantedGameplayEffectHandles;
    TArray<FGameplayAbilitySpecHandle>  GrantedAbilityHandles;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryChanged, const TArray<FInventorySlot>&, Items);

/**
 * Replicated inventory component with slot-based item management.
 *
 * Supports adding/removing items, active slot selection, per-item GAS
 * effect/ability grants, team-aware item spawning on drop, and
 * future-timeline item invalidation. Changes are broadcast via
 * OnInventoryChanged for UI updates.
 */
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SPLITSTREAM_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();
    virtual void BeginDestroy() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void UpdateEquippedItemMesh(const TArray<FInventorySlot>& InSlots);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Mesh")
    class UStaticMeshComponent* EquippedItemMeshComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<UItemBase*> DefaultItemAssets;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryChanged OnInventoryChanged;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 SlotCount = 10;

    UPROPERTY(ReplicatedUsing = OnRep_Slots, VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TArray<FInventorySlot> Slots;

    UPROPERTY(ReplicatedUsing = OnRep_ActiveSlotIndex, VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 ActiveSlotIndex = 0;

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SetActiveSlot(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(UItemBase* ItemAsset, FGuid ItemInstanceID);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RemoveItem(int32 Index);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    FInventorySlot GetActiveItem() const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    FInventorySlot CreateSlot(UItemBase* ItemAsset, FGuid InstanceID) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void DropActiveItem(FTransform DropTransform);

    UFUNCTION()
    void OnRep_Slots();

    UFUNCTION()
    void OnRep_ActiveSlotIndex();

    UFUNCTION(Server, Reliable)
    void ServerSetActiveSlot(int32 Index);

    UFUNCTION(Server, Reliable)
    void ServerDropActiveItem(FTransform DropTransform);

    UFUNCTION(Server, Reliable)
    void ServerAddItem(UItemBase* ItemAsset);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    TArray<FInventorySlot> GetSlots() const
    {
        return Slots;
    }

    UFUNCTION(BlueprintPure, Category = "Inventory")
    FGameplayTag GetTeamTag() const;

    // Register for future invalidation
    void RegisterFutureInstance(FGuid ItemInstanceID);

    // Remove by instance ID
    void RemoveItemByInstanceID(FGuid ItemInstanceID);

private:
    UFUNCTION()
    void HandleFutureItemInvalidated(FGuid InvalidID);

    TSet<FGuid> RegisteredFutureInstances;
};