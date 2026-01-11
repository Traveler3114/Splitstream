#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "UserSettingsSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FSavedKeybind
{
    GENERATED_BODY()

    UPROPERTY()
    FName ActionName;

    UPROPERTY()
    FKey Key;
};

UCLASS()
class SPLITSTREAM_API UUserSettingsSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TArray<FSavedKeybind> SavedKeybinds;

    UPROPERTY()
    float MouseSensitivity = 1.0f;
};