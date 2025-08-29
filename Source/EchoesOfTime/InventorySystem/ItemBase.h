#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ItemBase.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Keycard     UMETA(DisplayName = "Keycard"),
    Tool        UMETA(DisplayName = "Tool"),
    Gadget      UMETA(DisplayName = "Gadget"),
    Other       UMETA(DisplayName = "Other")
};

UCLASS(Blueprintable, Abstract)
class ECHOESOFTIME_API UItemBase : public UObject
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

    UPROPERTY(BlueprintReadWrite, Category = "Item")
    FGuid ItemInstanceID;


    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
    void OnEquipped(class AActor* Instigator);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
    void OnUsed(class AActor* Instigator);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
    void OnDropped(class AActor* Instigator, FVector DropLocation);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
    void OnDroppedWithTeam(AActor* Instigator, FGameplayTag TeamTag,FVector DropLocation);
};