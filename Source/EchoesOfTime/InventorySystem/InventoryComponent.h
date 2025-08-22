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
    TSubclassOf<UItemBase> ItemClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid ItemInstanceID;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryChanged, const TArray<FInventorySlot>&, Items);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ECHOESOFTIME_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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
    bool AddItem(TSubclassOf<UItemBase> ItemClass, FGuid ItemInstanceID);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RemoveItem(int32 Index);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    UItemBase* GetActiveItem() const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    UItemBase* CreateItemInstance(const FInventorySlot& Slot) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void DropActiveItem();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TSubclassOf<UItemBase> DefaultItemClass;

    UFUNCTION()
    void OnRep_Slots();

    UFUNCTION()
    void OnRep_ActiveSlotIndex();

    UFUNCTION(Server, Reliable)
    void ServerSetActiveSlot(int32 Index);

    UFUNCTION(Server, Reliable)
    void ServerDropActiveItem();

    UFUNCTION(Server, Reliable)
    void ServerAddItem(TSubclassOf<UItemBase> ItemClass);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    TArray<UItemBase*> GetItemInstances() const
    {
        TArray<UItemBase*> Result;
        for (const FInventorySlot& Slot : Slots)
        {
            if (Slot.ItemClass)
            {
                Result.Add(NewObject<UItemBase>(GetOwner(), Slot.ItemClass));
            }
        }
        return Result;
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