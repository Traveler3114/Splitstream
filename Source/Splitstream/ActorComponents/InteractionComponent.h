#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class UInventoryComponent;
class UCameraComponent;
class UItemBase;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SPLITSTREAM_API UInteractionComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UInteractionComponent();

    // --- Drop preview ghost ---
    UPROPERTY()
    UStaticMeshComponent* DropPreviewMesh = nullptr;
    UPROPERTY(EditDefaultsOnly)
    UMaterialInterface* DropGhostMaterial = nullptr;
    FTransform CachedDropTransform;
    bool bDropPreviewIsValid = false;

    // Drop preview workflow
    void StartDropPreview();
    void TickDropPreview(FVector CamLoc, FRotator CamRot);
    void StopDropPreview();
    void OnReleaseDropPreview(UInventoryComponent* Inventory);
    UStaticMesh* GetGhostMeshFromPickup(UItemBase* ItemAsset);

    // --- Interaction general ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractDistance = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float DropDistance = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractHighlightInterval = 0.1f;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    AActor* HighlightedActor = nullptr;

    TWeakObjectPtr<AActor> ProgressiveActor = nullptr;

    bool bIsHoldingInteract = false;
    float InteractHighlightTimer = 0.f;

    bool GetForwardTraceResult(FVector Start, FRotator Rotation, float TraceDistance,
        FHitResult& OutHit, FVector& OutTraceEnd) const;
    void UpdateInteractHighlight(FVector Start, FRotator Rotation);
    void HandleHoldInteractStart(AActor* Instigator, FVector Start, FRotator Rotation);
    void HandleHoldInteractStop(AActor* Instigator);
    void HandleInstantInteract(AActor* Instigator, FVector Start, FRotator Rotation);

    UFUNCTION(Server, Reliable)
    void ServerHandleInteract(AActor* TargetActor);
};