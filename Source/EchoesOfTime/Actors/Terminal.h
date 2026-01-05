#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IRepairable.h"
#include "Interfaces/IInteractable.h"
#include "Interfaces/IServerActionInterface.h"
#include "TimelineEra.h"
#include "Terminal.generated.h"

UCLASS()
class ECHOESOFTIME_API ATerminal : public AActor, public IRepairable, public IInteractable, public IServerActionInterface
{
    GENERATED_BODY()
public:
    ATerminal();

    // ----- IRepairable implementation -----
    virtual void RequestRepair_Implementation(AActor* RepairInstigator) override;
    virtual float GetRepairTime_Implementation() const override { return RepairTime; }
    virtual ETimelineEra GetTimelineEra_Implementation() const override { return TimelineEra; }
    virtual AActor* GetCompletionTarget_Implementation() const override { return CompletionTarget ? CompletionTarget : const_cast<ATerminal*>(this); }
    virtual FOnRepairRequested& GetOnRepairRequested() override { return OnRepairRequested; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair")
    float RepairTime = 5.0f;

    UPROPERTY(BlueprintAssignable, Category = "Repair")
    FOnRepairRequested OnRepairRequested;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Completion")
    AActor* CompletionTarget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* TerminalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MiniGame")
    TSubclassOf<class UNeonRunnerMiniGame> NeonRunnerMiniGameClass;

    void HandleMiniGameEnded_Internal(bool bWasVictory);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;

    virtual void ExecuteServerAction_Implementation(const FServerActionPayload& Payload) override;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated)
    bool bEnabled = true;

    UPROPERTY()
    UNeonRunnerMiniGame* MiniGameInstance = nullptr;

    UFUNCTION()
    void OnMiniGameEnded(bool bWasVictory);

private:
    TWeakObjectPtr<APlayerController> LastInteractingPC;
};