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

    UFUNCTION(Server, Reliable)
    void ServerLockPickConfirm(AActor* DoorActor, float Angle);
private:
    UPROPERTY()
    class ACharacterHUD* CharacterHUD;

    UPROPERTY()
    class UCharacterOverlay* CharacterOverlay;
};