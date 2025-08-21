#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ItemBase.generated.h"

/**
 * EItemType - Enum for item categories (expand as needed)
 */
UENUM(BlueprintType)
enum class EItemType : uint8
{
    Keycard     UMETA(DisplayName = "Keycard"),
    Tool        UMETA(DisplayName = "Tool"),
    Gadget      UMETA(DisplayName = "Gadget"),
    Other       UMETA(DisplayName = "Other")
};

/**
 * UItemBase - Abstract base class for inventory items
 */
UCLASS(Blueprintable, Abstract)
class ECHOESOFTIME_API UItemBase : public UObject
{
    GENERATED_BODY()
public:

    // Item display name
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText ItemName;

    // Short description
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText ItemDescription;

    // Icon for UI
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    UTexture2D* ItemIcon;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    UStaticMesh* ItemMesh;

    // Item type (Keycard, Tool, etc.)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EItemType ItemType;

    // Called when the item is equipped
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
    void OnEquipped(class AActor* Instigator);

    // Called when the item is used/activated
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
    void OnUsed(class AActor* Instigator);

    // Called when the item is dropped
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
    void OnDropped(class AActor* Instigator);
};