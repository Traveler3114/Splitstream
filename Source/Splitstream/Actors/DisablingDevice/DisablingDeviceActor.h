#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "TimelineEra.h"
#include "DisablingDeviceActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSoloDeviceDisabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeviceStateChanged, ADisablingDeviceActor*, Device);

UCLASS()
class SPLITSTREAM_API ADisablingDeviceActor : public AActor, public IInteractable
{
    GENERATED_BODY()
    
public:
    ADisablingDeviceActor();


    //UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
    //FLinearColor ManagerBaseColor = FLinearColor(0.1f, 0.4f, 0.9f, 1.f);

    //UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
    //FLinearColor SoloBaseColor = FLinearColor(0.9f, 0.15f, 0.15f, 1.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(BlueprintReadOnly, Replicated)
    FString SpawnLocationName;

    UPROPERTY(EditAnywhere, ReplicatedUsing=OnRep_DeviceState)
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, Replicated)
    bool bIsSolo = false;

    UPROPERTY(BlueprintAssignable, Category="Device")
    FOnSoloDeviceDisabled OnSoloDeviceDisabled; // Fires if solo device disabled

    UPROPERTY(BlueprintAssignable, Category="Device")
    FOnDeviceStateChanged OnDeviceStateChanged;

    /** Root Scene Component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Device")
    USceneComponent* SceneRootComp;

    /** Static Mesh for the device */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Device")
    UStaticMeshComponent* DeviceMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class UArrowComponent> ArrowComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device")
    class USearchComponent* SearchComponent;

    UFUNCTION(BlueprintCallable)
    void DisableDevice();

    UFUNCTION(BlueprintCallable)
    void SetIsSolo(bool bSolo);

    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void CancelInteract_Implementation(AActor* Interactor) override;
    virtual bool IsProgressiveInteract_Implementation() override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;

    UFUNCTION()
    virtual void OnSearchComplete();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Device")
    TArray<AActor*> CompletionTargets;

protected:
    virtual void BeginPlay() override;
    UFUNCTION() void OnRep_DeviceState();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


};