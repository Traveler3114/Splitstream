#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "DefaultSpectatorPawn.generated.h"

UCLASS()
class SPLITSTREAM_API ADefaultSpectatorPawn : public ASpectatorPawn
{
    GENERATED_BODY()

public:
    ADefaultSpectatorPawn();


    virtual void SetupPlayerInputComponent(UInputComponent* InInputComponent) override;
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
    // For spectating logic
    void SpectateNext();
    void SpectatePrevious();
    void SetSpectateTarget(int32 Index);
    void UpdateSpectateList();

protected:

    // Prevent movement (overwrite movement input handler)
    virtual void MoveForward(float);
    virtual void MoveRight(float);

    TArray<APlayerState*> SpectatablePlayers;
    int32 SpectateTargetIndex = 0;
};