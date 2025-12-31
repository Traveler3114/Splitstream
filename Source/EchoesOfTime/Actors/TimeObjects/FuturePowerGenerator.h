#pragma once

#include "CoreMinimal.h"
#include "Actors/PowerGenerator.h"
#include "Interfaces/IServerActionInterface.h"
#include "FuturePowerGenerator.generated.h"

UCLASS()
class ECHOESOFTIME_API AFuturePowerGenerator : public APowerGenerator, public IServerActionInterface
{
    GENERATED_BODY()

public:
    AFuturePowerGenerator();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual bool IsProgressiveInteract_Implementation() override;
    virtual void CancelInteract_Implementation(AActor* Interactor) override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;

    virtual void RequestRepair(AActor* RepairInstigator) override;

    // NEW: Universal server action interface implementation
    virtual void ExecuteServerAction_Implementation(const FServerActionPayload& Payload) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PastPowerGenerator")
    TSoftObjectPtr<class APastPowerGenerator> PastGenerator;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MiniGame")
    TSubclassOf<class UFirewallMiniGame> FirewallMiniGameClass;

protected:
    UFUNCTION()
    void HandlePastGeneratorCompleted(bool bCompleted);

    UFUNCTION()
    void OnMiniGameEnded(bool bWasVictory);

    UPROPERTY()
    class UFirewallMiniGame* MiniGameInstance = nullptr;

    TWeakObjectPtr<APlayerController> LastInteractingPC;

    // Enabled/disabled state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
    bool bEnabled = true;

    UPROPERTY(Replicated)
    bool bFutureInteracted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "EasterEgg")
	bool bEasterEggActive = false;
};