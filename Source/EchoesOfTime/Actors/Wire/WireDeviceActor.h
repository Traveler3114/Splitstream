#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WireActor.h"
#include "TimelineEra.h"
#include "Interfaces/IInteractable.h"
#include "WireDeviceActor.generated.h"

UCLASS()
class ECHOESOFTIME_API AWireDeviceActor : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    AWireDeviceActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* DeviceMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class UArrowComponent> ArrowComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device")
    class USearchComponent* SearchComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device")
    TSubclassOf<AWireActor> WireClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* WireDeviceSound = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<AWireActor*> WireActors;

    UPROPERTY(BlueprintReadOnly, Replicated)
    FString SpawnLocationName;

    UFUNCTION()
    void OnWireCut(AWireActor* CutWire);

	UFUNCTION()
	void OnSearchComplete();

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastPlayWireSound();

    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void CancelInteract_Implementation(AActor* Interactor) override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;

protected:
    virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};