#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HackComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHackComplete);

UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ECHOESOFTIME_API UHackComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHackComponent();

    // ============================================
    // Unreal Engine Overrides
    // ============================================
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ============================================
    // Configuration
    // ============================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hacking")
    float HackDuration = 10.f;

    // ============================================
    // Hack State
    // ============================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Hacking")
    bool bHackingInProgress = false;

    UPROPERTY(ReplicatedUsing = OnRep_Hacked, EditAnywhere, BlueprintReadWrite, Category = "Hacking")
    bool bHacked = false;

    // ============================================
    // Events
    // ============================================
    UPROPERTY(BlueprintAssignable, Category = "Hacking")
    FOnHackComplete OnHackComplete;

    // ============================================
    // Hack Actions
    // ============================================
    UFUNCTION(BlueprintCallable, Category = "Hacking")
    void StartHacking();

    UFUNCTION(BlueprintCallable, Category = "Hacking")
    void CancelHacking();

    UFUNCTION(BlueprintCallable, Category = "Hacking")
    void Interact(AActor* Interactor);

    UFUNCTION(BlueprintCallable, Category = "Hacking")
    void CancelInteract(AActor* Interactor);

    // ============================================
    // Query Functions
    // ============================================
    UFUNCTION(BlueprintCallable, Category = "Hacking")
    float GetHackProgress() const;

    // ============================================
    // Network RPCs
    // ============================================
    UFUNCTION(NetMulticast, Reliable)
    void MulticastResetHackElapsed();

    UFUNCTION()
    void OnRep_Hacked();

protected:
    // ============================================
    // Internal State
    // ============================================
    float HackElapsed = 0.f;
};