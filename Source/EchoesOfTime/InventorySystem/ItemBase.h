#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemBase.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Keycard     UMETA(DisplayName = "Keycard"),
    MoneyStack  UMETA(DisplayName = "MoneyStack"),
    Tool        UMETA(DisplayName = "Tool"),
    Gadget      UMETA(DisplayName = "Gadget"),
    Other       UMETA(DisplayName = "Other")
};

UCLASS(BlueprintType)
class ECHOESOFTIME_API UItemBase : public UDataAsset
{
    GENERATED_BODY()
public:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText ItemName;

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

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
    void OnEquipped(class AActor* Instigator);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
    void OnUsed(class AActor* Instigator);

    // InstanceID is passed in now:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
    void OnDropped(class AActor* Instigator, FGuid ItemInstanceID, FVector DropLocation);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
    void OnDroppedWithTeam(AActor* Instigator, FGuid ItemInstanceID, FGameplayTag TeamTag, FVector DropLocation);
};