#pragma once

#include "CoreMinimal.h"
#include "Actors/VentBase.h"
#include "PastVent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVentStateChanged, bool, bIsOpen);

UCLASS()
class ECHOESOFTIME_API APastVent : public AVentBase
{
    GENERATED_BODY()

public:
    APastVent();

    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(BlueprintAssignable, Category = "PastVent")
    FOnVentStateChanged OnVentStateChanged;

    void OnRep_OpenState() override;
};