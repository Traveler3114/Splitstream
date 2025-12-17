#pragma once

#include "CoreMinimal.h"
#include "Actors/DoorBase.h"
#include "PastDoor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDoorStateChanged, bool, bIsOpen, int32, Direction);

UCLASS()
class ECHOESOFTIME_API APastDoor : public ADoorBase
{
    GENERATED_BODY()

public:
    APastDoor();

    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(BlueprintAssignable, Category = "PastDoor")
    FOnDoorStateChanged OnDoorStateChanged;

    void OnRep_IsOpen() override;
};