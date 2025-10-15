#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

class UHorizontalBox;
class UCanvasPanel;
class UTextBlock;

UCLASS()
class ECHOESOFTIME_API UCharacterOverlay : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeDestruct() override;

    UPROPERTY(meta = (BindWidget))
    UHorizontalBox* InventoryBox;

    UPROPERTY(meta = (BindWidget))
    UCanvasPanel* CanvasPanel;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* status_txt;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* health_txt;

    UFUNCTION()
    void SetHealthText(float NewHealth);


    UPROPERTY(meta = (BindWidget))
    UTextBlock* ping_txt;

    UFUNCTION(BlueprintCallable)
    void SetPingText(float NewPing);

    UFUNCTION()
    void OnInventoryChanged(const TArray<FInventorySlot>& Items);

    UFUNCTION(BlueprintCallable)
    void SetStatusText(const FString& NewStatus);

    UPROPERTY()
    class UInventoryComponent* LinkedInventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<class UDetectionWidget> DetectionWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float TextureScale = 1.25f;

    UPROPERTY()
    TMap<AActor*, class UDetectionWidget*> GuardDetectionWidgets;

    UFUNCTION(BlueprintCallable)
    void UpdateDetectionWidgetForGuard(AActor* Guard, float Progress, bool bIsLocked, float AngleDegrees = 0.0f);
};