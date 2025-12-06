#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataAssets/ItemBase.h"
#include "InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UItemBase* ItemAsset = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid ItemInstanceID;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryChanged, const TArray<FInventorySlot>&, Items);

UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ECHOESOFTIME_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    // ============================================
    // Unreal Engine Overrides
    // ============================================
    virtual void BeginPlay() override;
    virtual void BeginDestroy() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ============================================
    // Configuration
    // ============================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    UItemBase* DefaultItemAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 SlotCount = 10;

    // ============================================
    // Inventory State
    // ============================================
    UPROPERTY(ReplicatedUsing = OnRep_Slots, VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TArray<FInventorySlot> Slots;

    UPROPERTY(ReplicatedUsing = OnRep_ActiveSlotIndex, VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 ActiveSlotIndex = 0;

    // ============================================
    // Events
    // ============================================
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryChanged OnInventoryChanged;

    // ============================================
    // Inventory Management
    // ============================================
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SetActiveSlot(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(UItemBase* ItemAsset, FGuid ItemInstanceID);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RemoveItem(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void DropActiveItem(FVector DropLocation);

    void RemoveItemByInstanceID(FGuid ItemInstanceID);
    void RegisterFutureInstance(FGuid ItemInstanceID);

    // ============================================
    // Query Functions
    // ============================================
    UFUNCTION(BlueprintPure, Category = "Inventory")
    FInventorySlot GetActiveItem() const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    FInventorySlot CreateSlot(UItemBase* ItemAsset, FGuid InstanceID) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    TArray<FInventorySlot> GetSlots() const { return Slots; }

    UFUNCTION(BlueprintPure, Category = "Inventory")
    FGameplayTag GetTeamTag() const;

    // ============================================
    // Network RPCs
    // ============================================
    UFUNCTION(Server, Reliable)
    void ServerSetActiveSlot(int32 Index);

    UFUNCTION(Server, Reliable)
    void ServerDropActiveItem(FVector DropLocation);

    UFUNCTION(Server, Reliable)
    void ServerAddItem(UItemBase* ItemAsset);

    UFUNCTION()
    void OnRep_Slots();

    UFUNCTION()
    void OnRep_ActiveSlotIndex();

private:
    // ============================================
    // Future Item Invalidation
    // ============================================
    TSet<FGuid> RegisteredFutureInstances;

    UFUNCTION()
    void HandleFutureItemInvalidated(FGuid InvalidID);
};