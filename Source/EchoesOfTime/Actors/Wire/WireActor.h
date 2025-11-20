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
    Yellow
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWireCutSignature, AWireActor*, Wire);

UCLASS()
class ECHOESOFTIME_API AWireActor : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    AWireActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* WireMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire")
    EWireColor WireColor = EWireColor::Red;

    UPROPERTY(ReplicatedUsing = OnRep_CutState, BlueprintReadOnly)
    bool bIsCut = false;

    UPROPERTY(BlueprintAssignable, Category = "Wire")
    FOnWireCutSignature OnWireCut;

    virtual void Interact_Implementation(AActor* InteractingActor) override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;

    UFUNCTION()
    void OnRep_CutState();

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};