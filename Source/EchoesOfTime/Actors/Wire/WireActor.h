#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "TimelineEra.h"
#include "WireActor.generated.h"

UENUM(BlueprintType)
enum class EWireColor : uint8
{
    Red,
    Green,
    Blue,
    Yellow,
    Orange,
    Purple
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWireCutSignature, AWireActor*, Wire);

UCLASS()
class ECHOESOFTIME_API AWireActor : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    AWireActor();

    UFUNCTION(BlueprintCallable, Category = "Wire")
    void ApplyWireColor();

    UFUNCTION(BlueprintPure, Category = "Wire")
    FLinearColor GetWireLinearColor() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* WireMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device")
    class USearchComponent* SearchComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire")
    EWireColor WireColor = EWireColor::Red;

    UPROPERTY(ReplicatedUsing = OnRep_CutState, BlueprintReadOnly)
    bool bIsCut = false;

    UPROPERTY(BlueprintAssignable, Category = "Wire")
    FOnWireCutSignature OnWireCut;

    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void CancelInteract_Implementation(AActor* Interactor) override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;

    UFUNCTION()
    void OnRep_CutState();

    UFUNCTION()
    virtual void OnSearchComplete();

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};