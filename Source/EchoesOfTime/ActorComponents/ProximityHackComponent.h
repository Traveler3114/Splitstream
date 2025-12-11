// ProximityHackComponent.h
#pragma once

#include "CoreMinimal.h"
#include "HackComponent.h"
#include "GameplayTagContainer.h"
#include "ProximityHackComponent.generated.h"

class ACharacter;
class UAbilitySystemComponent;
class UWidgetComponent;
class UProximityHackWidget;
class USphereComponent;

/**
 * Automatic, proximity-based hacking that reuses UHackComponent logic.
 * Attach this to the guard. It:
 *  - Automatically starts hacking when a player enters radius.
 *  - Only allows ONE active hacker at a time (first in wins).
 *  - Does not require abilities, input, or HUD widgets.
 *  - Shows a world-space progress bar only to the player who is currently hacking.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ECHOESOFTIME_API UProximityHackComponent : public UHackComponent
{
    GENERATED_BODY()

public:
    UProximityHackComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** Radius in which players can start / maintain hacking. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Hack")
    float ProximityRadius = 600.f;

    /** Tag applied to the single active proximity hacker while hacking. Optional. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Hack|GAS")
    FGameplayTag ProximityHackingTag;

    /** World-space widget class that shows progress over the guard. Should derive from UProximityHackWidget. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Hack|UI")
    TSubclassOf<UProximityHackWidget> ProximityHackWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Proximity Hack|Reward")
    class UItemBase* RewardItem = nullptr;
protected:
    UFUNCTION()
    void HandleHackComplete();
    /** Overlap sphere used to detect nearby players (server authoritative). */
    UPROPERTY()
    USphereComponent* DetectionSphere = nullptr;

    /** Players currently inside the detection sphere (server only). */
    UPROPERTY()
    TArray<TWeakObjectPtr<ACharacter>> PlayersInRange;

    /** Player currently hacking this guard via proximity (server only, replicated for debug/UX). */
    UPROPERTY(Replicated)
    ACharacter* ActiveHacker = nullptr;

    /** Widget component attached to the guard. */
    UPROPERTY()
    UWidgetComponent* ProximityWidgetComponent = nullptr;

    /** The actual widget instance we call SetHackProgress() on. */
    UPROPERTY()
    UProximityHackWidget* ProximityHackWidgetInstance = nullptr;

    /** Overlap handlers (server side). */
    UFUNCTION()
    void OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    /** Internal helpers (server side). */
    void Server_UpdateProximity();          // uses PlayersInRange, no radius search
    void StartHackingForPlayer(ACharacter* NewHacker);
    void CancelHackingForPlayer();
    ACharacter* FindFirstPlayerInRange() const;

    /** Should this local machine show the widget? (true only if local pawn == ActiveHacker and hacking in progress). */
    bool IsLocallyViewedByActiveHacker() const;

    /** GAS tag application helpers (server side). */
    void ApplyProximityTagTo(ACharacter* Player);
    void RemoveProximityTagFrom(ACharacter* Player);
};