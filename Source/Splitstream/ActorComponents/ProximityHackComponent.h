// ProximityHackComponent.h
#pragma once

#include "CoreMinimal.h"
#include "HackComponent.h"
#include "GameplayTagContainer.h"
#include "ActiveGameplayEffectHandle.h"
#include "ProximityHackComponent.generated.h"

class ACharacter;
class UAbilitySystemComponent;
class UWidgetComponent;
class UProximityHackWidget;
class USphereComponent;
class UItemBase;
struct FGameplayEffectRemovalInfo;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SPLITSTREAM_API UProximityHackComponent : public UHackComponent
{
    GENERATED_BODY()

public:
    UProximityHackComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Hack")
    float ProximityRadius = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Hack|GAS")
    FGameplayTag ProximityHackingTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Hack|UI")
    TSubclassOf<UProximityHackWidget> ProximityHackWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Hack|Reward")
    UItemBase* RewardItem = nullptr;

protected:
    UFUNCTION()
    void HandleHackComplete();

    void OnTimerRemoved(const FGameplayEffectRemovalInfo& RemovalInfo);

    UPROPERTY()
    USphereComponent* DetectionSphere = nullptr;

    UPROPERTY()
    TArray<TWeakObjectPtr<ACharacter>> PlayersInRange;

    UPROPERTY(Replicated)
    ACharacter* ActiveHacker = nullptr;

    UPROPERTY()
    UWidgetComponent* ProximityWidgetComponent = nullptr;

    UPROPERTY()
    UProximityHackWidget* ProximityHackWidgetInstance = nullptr;

    /** Are we currently draining HackElapsed back to zero? Shared so client drains too. */
    UPROPERTY(Replicated)
    bool bDrainingHack = false;

    UFUNCTION()
    void OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void Server_UpdateProximity();
    void StartHackingForPlayer(ACharacter* NewHacker);
    void BeginDrainForActiveHacker();
    void FullyCancelAndReset();
    ACharacter* FindFirstPlayerInRange() const;

    bool IsLocallyViewedByActiveHacker() const;

    void ApplyProximityTagTo(ACharacter* Player);
    void RemoveProximityTagFrom(ACharacter* Player);

    /** Apply or re-apply the GE_Timer on the active hacker's ASC */
    void ApplyTimerEffect();

    TWeakObjectPtr<APlayerController> CachedFirstPC;

    // Local timer state for fill/drain visual (GE_Timer handles server-side completion)
    float HackElapsed = 0.f;
    bool bHackingInProgress = false;
    FActiveGameplayEffectHandle TimerEffectHandle;

    void CancelHacking();
    UFUNCTION(NetMulticast, Reliable)
    void MulticastResetHackElapsed();
};
