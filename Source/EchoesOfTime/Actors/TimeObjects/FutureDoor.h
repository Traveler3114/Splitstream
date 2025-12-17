#pragma once

#include "CoreMinimal.h"
#include "Actors/DoorBase.h"
#include "FutureDoor.generated.h"

UCLASS()
class ECHOESOFTIME_API AFutureDoor : public ADoorBase
{
    GENERATED_BODY()

public:
    AFutureDoor();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PastDoor")
    TSoftObjectPtr<class APastDoor> PastDoor;

    void OnRep_IsOpen() override;

    UFUNCTION()
    void HandlePastDoorStateChanged(bool bPastIsOpen, int32 Direction);
};