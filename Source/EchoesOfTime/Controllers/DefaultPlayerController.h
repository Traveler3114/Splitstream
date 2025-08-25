#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DefaultPlayerController.generated.h"

UCLASS()
class ECHOESOFTIME_API ADefaultPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    ADefaultPlayerController();

    virtual void BeginPlay() override;

private:
    UPROPERTY()
    class ACharacterHUD* CharacterHUD;

    UPROPERTY()
    class UCharacterOverlay* CharacterOverlay;
};