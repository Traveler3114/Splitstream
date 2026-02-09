#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IRepairable.h"
#include "TimelineEra.h"
#include "Interfaces/IInteractable.h"
#include "PowerGenerator.generated.h"

UCLASS()
class SPLITSTREAM_API APowerGenerator : public AActor, public IRepairable, public IInteractable
{
    GENERATED_BODY()
public:
    APowerGenerator();

    // IRepairableInterface
    virtual void RequestRepair_Implementation(AActor* RepairInstigator) override;
    virtual float GetRepairTime_Implementation() const override { return RepairTime; }
    virtual ETimelineEra GetTimelineEra_Implementation() const override { return TimelineEra; }
    virtual AActor* GetCompletionTarget_Implementation() const override { return CompletionTarget ? CompletionTarget : const_cast<APowerGenerator*>(this); }
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
    UStaticMeshComponent* GeneratorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device")
    class USearchComponent* SearchComponent;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void CancelInteract_Implementation(AActor* Interactor) override;
    virtual bool IsProgressiveInteract_Implementation() override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;

    /** Called when search is complete (searched by player) */
    UFUNCTION()
    virtual void OnSearchComplete();
};