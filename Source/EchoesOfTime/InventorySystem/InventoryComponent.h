#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemBase.h"
#include "InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UItemBase* ItemAsset = nullptr; // Pointer to DataAsset

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
    virtual void BeginDestroy() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    UItemBase* DefaultItemAsset;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryChanged OnInventoryChanged;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 SlotCount = 9;

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
    void DropActiveItem(FVector DropLocation);

    UFUNCTION()
    void OnRep_Slots();

    UFUNCTION()
    void OnRep_ActiveSlotIndex();

    UFUNCTION(Server, Reliable)
    void ServerSetActiveSlot(int32 Index);

    UFUNCTION(Server, Reliable)
    void ServerDropActiveItem(FVector DropLocation);

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