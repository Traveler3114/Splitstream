#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "TimelineEra.h"
#include "LeverActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLeverInteracted, ALeverActor*, Lever);

UCLASS()
class SPLITSTREAM_API ALeverActor : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    ALeverActor();
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever")
    USceneComponent* SceneRootComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever")
    UStaticMeshComponent* LeverMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever")
    UStaticMeshComponent* LeverBaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class UArrowComponent> ArrowComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lever")
    bool bIsSolo = false;

    UPROPERTY(BlueprintReadOnly, Replicated)
    FString SpawnLocationName;

    UPROPERTY(ReplicatedUsing = OnRep_Activated)
    bool bActivated = false;

    // Event for manager to bind to!
    UPROPERTY(BlueprintAssignable, Category = "Lever")
    FOnLeverInteracted OnLeverInteracted;

    // Interaction interface
    virtual void Interact_Implementation(AActor* Interactor) override;

    virtual void SetHighlighted_Implementation(bool bHighlight) override;

    UFUNCTION()
    void OnRep_Activated();

    UFUNCTION(BlueprintImplementableEvent, Category = "Lever")
    void OnActivated();

    UFUNCTION(BlueprintImplementableEvent, Category = "Lever")
    void OnDeactivated();

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};