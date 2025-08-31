#pragma once

#include "CoreMinimal.h"
#include "Actors/DoubleDoorBase.h"
#include "FutureDoubleDoor.generated.h"

UCLASS()
class ECHOESOFTIME_API AFutureDoubleDoor : public ADoubleDoorBase
{
    GENERATED_BODY()

public:
    AFutureDoubleDoor();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PastDoubleDoor")
    TSoftObjectPtr<class APastDoubleDoor> PastDoubleDoor;

    void OnRep_IsOpen() override;

    UFUNCTION()
    void HandlePastDoubleDoorStateChanged(bool bPastIsOpen);
};