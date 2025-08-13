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
#include "GameStates/LobbyGameState.h"
#include "TimerManager.h"

ALobbyGameMode::ALobbyGameMode()
{
    // Set the default GameState class to use our new LobbyGameState
    GameStateClass = ALobbyGameState::StaticClass();
}

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
    int32 PlatformIndex = -1;
    for (int32 i = 0; i < LobbyPlatforms.Num(); ++i)
    {
        ALobbyPlatformActor* Platform = Cast<ALobbyPlatformActor>(LobbyPlatforms[i]);
        if (!Platform || Platform->OccupyingPawn)
        {
            continue;
        }

        APawn* SpawnedPawn = Platform->SpawnCharacterAtPlatform(NewPlayer);
        if (PlayerState)
        {
            PlayerState->AssignedPlatform = Platform;
            PlatformIndex = i;
            
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

    // Add player to the new GameState roster for improved networking
    if (ALobbyGameState* LobbyGS = GetGameState<ALobbyGameState>())
    {
        if (PlayerState)
        {
            FString PlayerName = PlayerState->GetPlayerName();
            int32 PlayerId = PlayerState->GetPlayerId();
            
            LobbyGS->AddPlayerToRoster(PlayerId, PlayerName, PlatformIndex);
            
            // Sync initial team tag
            FGameplayTag TeamTag = PlayerState->TeamTag;
            if (TeamTag.IsValid())
            {
                LobbyGS->UpdatePlayerTeam(PlayerId, TeamTag);
            }
            
            UE_LOG(LogTemp, Log, TEXT("Added player %s to lobby GameState roster"), *PlayerName);
        }
    }
}

void ALobbyGameMode::Logout(AController* Exiting)
{
    // Remove player from GameState roster before calling super
    if (ALobbyGameState* LobbyGS = GetGameState<ALobbyGameState>())
    {
        if (ADefaultPlayerState* PlayerState = Cast<ADefaultPlayerState>(Exiting->PlayerState))
        {
            int32 PlayerId = PlayerState->GetPlayerId();
            LobbyGS->RemovePlayerFromRoster(PlayerId);
            
            UE_LOG(LogTemp, Log, TEXT("Removed player %s from lobby GameState roster"), *PlayerState->GetPlayerName());
        }
    }
    
    Super::Logout(Exiting);
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

    // Use new GameState for more efficient ready checking
    if (ALobbyGameState* LobbyGS = GetGameState<ALobbyGameState>())
    {
        bEveryoneReady = LobbyGS->AreAllPlayersReady();
    }
    else
    {
        // Fallback: Evaluate readiness from PlayerState
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

void ALobbyGameMode::SyncPlayerStateToGameState(ADefaultPlayerState* PlayerState)
{
    if (!PlayerState)
    {
        return;
    }

    if (ALobbyGameState* LobbyGS = GetGameState<ALobbyGameState>())
    {
        int32 PlayerId = PlayerState->GetPlayerId();
        
        // Update ready state
        LobbyGS->UpdatePlayerReady(PlayerId, PlayerState->bIsReady);
        
        // Update team tag
        if (PlayerState->TeamTag.IsValid())
        {
            LobbyGS->UpdatePlayerTeam(PlayerId, PlayerState->TeamTag);
        }
    }
}

void ALobbyGameMode::StartGame()
{
    // Set lobby to countdown phase
    if (ALobbyGameState* LobbyGS = GetGameState<ALobbyGameState>())
    {
        LobbyGS->SetLobbyPhase(ELobbyPhase::Countdown);
        LobbyGS->StartCountdown(5.0f); // 5 second countdown
    }

    // 1) Ask all clients to show a loading UI
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(It->Get()))
        {
            PC->OnStartGame(); // Client RPC already in your project
        }
    }

    // 2) Seamless server travel to the game map (listen server) after a delay
    FTimerHandle TravelTimerHandle;
    GetWorldTimerManager().SetTimer(TravelTimerHandle, [this]()
    {
        if (ALobbyGameState* LobbyGS = GetGameState<ALobbyGameState>())
        {
            LobbyGS->SetLobbyPhase(ELobbyPhase::Traveling);
        }
        
        const FString MapPath = TEXT("/Game/Maps/TestMap?listen");
        GetWorld()->ServerTravel(MapPath);
    }, 5.0f, false);
}