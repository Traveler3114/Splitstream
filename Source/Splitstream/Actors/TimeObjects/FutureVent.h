#pragma once

#include "CoreMinimal.h"
#include "Actors/VentBase.h"
#include "FutureVent.generated.h"

UCLASS()
class ECHOESOFTIME_API AFutureVent : public AVentBase
{
    GENERATED_BODY()

public:
    AFutureVent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void CancelInteract_Implementation(AActor* Interactor) override;
	virtual void SetHighlighted_Implementation(bool bHighlight) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PastVent")
    TSoftObjectPtr<class APastVent> PastVent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vent")
    bool bIsInteractable = false;

    void OnRep_OpenState() override;

    UFUNCTION()
    void HandlePastVentStateChanged(bool bPastIsOpen);
};