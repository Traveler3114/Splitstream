#include "LobbyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/LobbyPlatformActor.h"
#include "Controllers/LobbyPlayerController.h"
#include "Engine/Engine.h"
#include "Widgets/Lobby/LobbyUI.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameplayTagContainer.h"
#include "DefaultPlayerState.h"
#include "Components/WidgetComponent.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    if (LobbyPlatforms.Num() == 0)
    {
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALobbyPlatformActor::StaticClass(), LobbyPlatforms);
    }

    ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(NewPlayer);
    ADefaultPlayerState* PlayerState = Cast<ADefaultPlayerState>(NewPlayer->PlayerState);

    if (PlayerState)
    {
        PlayerState->OnPlayerReady.AddDynamic(this, &ALobbyGameMode::CheckAllPlayersReady);
    }

    // Find the first available platform
    for (AActor* Actor : LobbyPlatforms)
    {
        ALobbyPlatformActor* Platform = Cast<ALobbyPlatformActor>(Actor);
        if (!Platform || Platform->OccupyingPawn)
        {
            continue;
        }

        APawn* SpawnedPawn = Platform->SpawnCharacterAtPlatform(NewPlayer);
        if (PlayerState)
        {
            PlayerState->AssignedPlatform = Platform;
            Platform->OnKickRequested.AddDynamic(this, &ALobbyGameMode::HandleKickRequestedFromPlatform);

            // Set default team tag on PlayerState
            FGameplayTag DefaultTeamTag = FGameplayTag::RequestGameplayTag(FName("Team.Future"));
            PlayerState->ServerSetTeamTag(DefaultTeamTag);

            // Initialize the widget on the server immediately (name, kick, etc.)
            PlayerState->RefreshLobbyInfoUI();
        }
        break; // Exit after assigning the first available platform
    }
}