#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CharacterOverlay.h"
#include "CharacterHUD.generated.h"

UCLASS()
class ECHOESOFTIME_API ACharacterHUD : public AHUD
{
    GENERATED_BODY()

public:
    FDelegateHandle IllegalTagDelegateHandle;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UCharacterOverlay> CharacterOverlayClass;

    UPROPERTY()
    UCharacterOverlay* CharacterOverlay;

    UFUNCTION(BlueprintCallable, Category = "UI")
    void AddCharacterOverlay();

	void BindTags(APawn* Pawn);

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};