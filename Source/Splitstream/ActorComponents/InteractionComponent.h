#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class UInventoryComponent;
class UCameraComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPLITSTREAM_API UInteractionComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UInteractionComponent();

    // How far can you interact?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractDistance = 150.f;

    // How often to update highlight (suggest: call from Tick on owner Pawn)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractHighlightInterval = 0.1f;

    // Which actor is currently highlighted? (for UI/feedback/etc)
    UPROPERTY(BlueprintReadOnly, Category="Interaction")
    AActor* HighlightedActor = nullptr;

    // Used for progressive interactions (e.g. hack, lockpick)
    TWeakObjectPtr<AActor> ProgressiveActor = nullptr;

    bool bIsHoldingInteract = false;
    float InteractHighlightTimer = 0.f;

    /** Performs a forward trace and returns what was hit. */
    bool GetForwardTraceResult(
        FVector Start, FRotator Rotation, float TraceDistance,
        FHitResult& OutHit, FVector& OutTraceEnd) const;

    /** Call this to update highlight state. Should be called every InteractHighlightInterval seconds. */
    void UpdateInteractHighlight(
        FVector Start, FRotator Rotation);

    /** Call this when 'Hold Interaction' input begins. */
    void HandleHoldInteractStart(
        AActor* Instigator, FVector Start, FRotator Rotation);

    /** Call this when 'Hold Interaction' input stops. */
    void HandleHoldInteractStop(
        AActor* Instigator);

    /**
     * Call this for instant/tap interaction.
     *
     * @param Instigator  The actor initiating interaction (the pawn usually).
     * @param Start/Rotation   Where to trace from.
     * @param ServerInteractCallback Called with the interacted actor. You MUST forward this to your ServerHandleInteract RPC. On the server, optionally may be null.
     */
    void HandleInstantInteract(
        AActor* Instigator, FVector Start, FRotator Rotation,
        TFunction<void(AActor* Target)> ServerInteractCallback = nullptr);


        // In InteractionComponent.h
    void DropEquippedItem(UInventoryComponent* InventoryComponent, UCameraComponent* CameraComponent, float TraceDistance = -1.f);
};