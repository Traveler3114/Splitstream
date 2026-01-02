#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DataAssets/AbilitySets/AbilityInputSet.h"
#include "GameplayAbilitySpec.h"
#include "ItemBase.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
    KeycardL1     UMETA(DisplayName = "KeycardL1"),
    KeycardL2     UMETA(DisplayName = "KeycardL2"),
    Fingerprint   UMETA(DisplayName = "Fingerprint"),
    MoneyBag      UMETA(DisplayName = "MoneyBag"),
    Tool          UMETA(DisplayName = "Tool"),
    Gadget        UMETA(DisplayName = "Gadget"),
    Pistol        UMETA(DisplayName = "Pistol"),
    PowerCell     UMETA(DisplayName = "PowerCell"),
    BodyBag       UMETA(DisplayName = "BodyBag"),
    EMP           UMETA(DisplayName = "EMP"),
    Other         UMETA(DisplayName = "Other")
};

UCLASS(BlueprintType)
class ECHOESOFTIME_API UItemBase : public UDataAsset
{
    GENERATED_BODY()
public:

    // --- Basic Item Data ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText ItemName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    int32 Value = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText ItemDescription;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    UTexture2D* ItemIcon;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EItemType ItemType;

    // In ItemBase.h, add:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Pickup")
    TSubclassOf<class AItemPickup> ItemPickupToSpawn;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Pickup")
    TSubclassOf<class APastItemPickup> PastItemPickupToSpawn;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Pickup")
    TSubclassOf<class AFutureItemPickup> FutureItemPickupToSpawn;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    bool bRemoveFromInventoryOnUse = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    bool bEnablePhysicsOnDrop = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    bool bApplyGameplayEffectsPassively = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    bool bApplyGameplayAbilitiesPassively = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    bool bAlertGuardsWhenSeen = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    float DropImpulseStrength = 500.f;

    // --- Ownership (optional, e.g. for fingerprints) ---
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ownership")
    class ACivilianCharacter* OwnerCivilian;

    // --- Effects (optional) ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
    TArray<TSubclassOf<class UGameplayEffect>> GrantedGameplayEffects;

    // --- Abilities (optional) ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
    UAbilityInputSet* AbilitySet;

    // --- Core API ---
    virtual void OnEquipped(class AActor* Instigator);
    virtual void OnUnequipped(class AActor* Instigator);
    virtual void OnAddedToInventory(AActor* Instigator);
    virtual void OnRemovedFromInventory(AActor* Instigator);

    void OnUsed(class AActor* Instigator, FGuid ItemInstanceID);

    virtual void OnDropped(class AActor* Instigator, FGuid ItemInstanceID, FVector DropLocation);
    virtual void OnDroppedWithTeam(class AActor* Instigator, FGuid ItemInstanceID, FGameplayTag TeamTag, FVector DropLocation);

    // NEW: Per-instance effect/ability handling
    void GrantEffectsTo(AActor* Instigator, TArray<FActiveGameplayEffectHandle>& OutHandles) const;
    void GrantAbilitiesTo(AActor* Instigator, TArray<FGameplayAbilitySpecHandle>& OutHandles) const;
    void RemoveGrantedEffectsFrom(AActor* Instigator, const TArray<FActiveGameplayEffectHandle>& Handles) const;
    void RemoveGrantedAbilitiesFrom(AActor* Instigator, const TArray<FGameplayAbilitySpecHandle>& Handles) const;
};