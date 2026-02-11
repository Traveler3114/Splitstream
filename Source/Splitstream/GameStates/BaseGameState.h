#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BaseGameState.generated.h"

/**
 * Base game state shared by all game modes (Lobby, Default, etc.).
 * Holds common map path configuration for main menu and lobby navigation.
 */
UCLASS()
class SPLITSTREAM_API ABaseGameState : public AGameState
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base|Maps")
    FString MainMenuMapPath = TEXT("/Game/Maps/MainMenuMap");

    UPROPERTY(EditDefaultsOnly, Category = "Maps")
    FString LobbyMapPath = TEXT("/Game/Maps/LobbyMap");
};