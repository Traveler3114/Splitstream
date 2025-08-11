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

void ALobbyGameMode::BeginPlay()
{
    Super::BeginPlay();
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALobbyPlatformActor::StaticClass(), LobbyPlatforms);
}

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
            // In PostLogin or wherever you assign the platform:
            Platform->OnKickRequestedPlatform.AddDynamic(this, &ALobbyGameMode::HandleKickRequestedFromPlatform);

            // Set default team tag on PlayerState
            FGameplayTag DefaultTeamTag = FGameplayTag::RequestGameplayTag(FName("Team.Future"));
            PlayerState->ServerSetTeamTag(DefaultTeamTag);

            // Initialize the widget on the server immediately (name, kick, etc.)
            PlayerState->RefreshLobbyInfoUI();
        }
        break; // Exit after assigning the first available platform
    }
}

void ALobbyGameMode::HandleKickRequestedFromPlatform(ALobbyPlatformActor* Platform)
{
    // Iterate through all player controllers in the world
    for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(Iterator->Get());
        if (PC)
        {
            // You can now access each PC here
            // For example, check if this PC is associated with the given Platform
            ADefaultPlayerState* PlayerState = Cast<ADefaultPlayerState>(PC->PlayerState);
            if (PlayerState && PlayerState->AssignedPlatform == Platform)        
            {
				KickPlayer(PC);
            }
        }
    }
}

void ALobbyGameMode::CheckAllPlayersReady()
{
    bool bEveryoneReady = true;

    // Evaluate readiness from PlayerState
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(It->Get());
        if (!PC) { bEveryoneReady = false; break; }

        ADefaultPlayerState* PlayerState = Cast<ADefaultPlayerState>(PC->PlayerState);
        if (!PlayerState || !PlayerState->bIsReady)
        {
            bEveryoneReady = false;
            break;
        }
    }

    // Tell the host's client to toggle the Start button
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(It->Get());
        if (PC && PC->IsLocalController() && PC->HasAuthority())
        {
            PC->ClientSetStartButtonEnabled(bEveryoneReady);
            break; // only the host needs this
        }
    }
}

void ALobbyGameMode::StartGame()
{
    // 1) Ask all clients to show a loading UI
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(It->Get()))
        {
            PC->OnStartGame(); // Client RPC already in your project
        }
    }

    // 2) Seamless server travel to the game map (listen server)
    const FString MapPath = TEXT("/Game/Maps/TestMap?listen");
    GetWorld()->ServerTravel(MapPath);
}