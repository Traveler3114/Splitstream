#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ItemBase.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
    KeycardL1     UMETA(DisplayName = "KeycardL1"),
    KeycardL2     UMETA(DisplayName = "KeycardL2"),
    Fingerprint   UMETA(DisplayName = "Fingerprint"),
    MoneyStack    UMETA(DisplayName = "MoneyStack"),
    Tool          UMETA(DisplayName = "Tool"),
    Gadget        UMETA(DisplayName = "Gadget"),
    Pistol        UMETA(DisplayName = "Pistol"),
    Other         UMETA(DisplayName = "Other")
};

UCLASS(BlueprintType)
class ECHOESOFTIME_API UItemBase : public UDataAsset
{
    GENERATED_BODY()
public:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText ItemName;


    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    int32 Value = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText ItemDescription;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    UTexture2D* ItemIcon;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    UStaticMesh* ItemMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EItemType ItemType;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FVector PickupMeshScale = FVector(1.0f);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FRotator PickupMeshRotation = FRotator(0.0f);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    bool bEnablePhysicsOnDrop = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    float DropImpulseStrength = 500.f; // Or whatever default!

    virtual void OnEquipped(class AActor* Instigator);

    virtual void OnUnequipped(class AActor* Instigator);

    virtual void OnUsed(class AActor* Instigator);

    virtual void OnDropped(class AActor* Instigator, FGuid ItemInstanceID, FVector DropLocation);

    virtual void OnDroppedWithTeam(AActor* Instigator, FGuid ItemInstanceID, FGameplayTag TeamTag, FVector DropLocation);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
    TArray<TSubclassOf<class UGameplayEffect>> GrantedGameplayEffects;

    UPROPERTY(Transient)
    TArray<struct FActiveGameplayEffectHandle> GrantedGameplayEffectHandles;

protected:
    void RemoveGrantedGameplayEffects(AActor* Instigator);
};