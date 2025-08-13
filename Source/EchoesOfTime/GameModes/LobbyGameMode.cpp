#include "LobbyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/LobbyPlatformActor.h"
#include "Controllers/LobbyPlayerController.h"
#include "GameStates/LobbyGameState.h"
#include "Engine/Engine.h"
#include "Widgets/Lobby/LobbyUI.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameplayTagContainer.h"
#include "DefaultPlayerState.h"
#include "Components/WidgetComponent.h"

ALobbyGameMode::ALobbyGameMode()
{
    // Set our custom GameState class
    GameStateClass = ALobbyGameState::StaticClass();
}

void ALobbyGameMode::InitGameState()
{
    Super::InitGameState();
    LobbyGameState = Cast<ALobbyGameState>(GameState);
}

void ALobbyGameMode::BeginPlay()
{
    Super::BeginPlay();
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALobbyPlatformActor::StaticClass(), LobbyPlatforms);
    
    UE_LOG(LogLobby, Log, TEXT("LobbyGameMode initialized with %d platforms"), LobbyPlatforms.Num());
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
        // Legacy delegate binding for backward compatibility
        PlayerState->OnPlayerReady.AddDynamic(this, &ALobbyGameMode::CheckAllPlayersReady);
    }

    // Find the first available platform using index system
    int32 AvailablePlatformIndex = FindAvailablePlatformIndex();
    if (AvailablePlatformIndex != -1)
    {
        ALobbyPlatformActor* Platform = GetPlatformByIndex(AvailablePlatformIndex);
        if (Platform)
        {
            APawn* SpawnedPawn = Platform->SpawnCharacterAtPlatform(NewPlayer);
            if (PlayerState)
            {
                PlayerState->AssignedPlatform = Platform;
                Platform->OnKickRequestedPlatform.AddDynamic(this, &ALobbyGameMode::HandleKickRequestedFromPlatform);

                // Set default team tag on PlayerState
                FGameplayTag DefaultTeamTag = FGameplayTag::RequestGameplayTag(FName("Team.Future"));
                PlayerState->ServerSetTeamTag(DefaultTeamTag);

                // Add player to GameState roster
                if (LobbyGameState)
                {
                    LobbyGameState->AddPlayerToRoster(PlayerState, AvailablePlatformIndex);
                }

                // Initialize the widget on the server immediately (name, kick, etc.)
                PlayerState->RefreshLobbyInfoUI();
                
                UE_LOG(LogLobby, Log, TEXT("Player %s assigned to platform index %d"), 
                       *PlayerState->GetPlayerName(), AvailablePlatformIndex);
            }
        }
    }
    else
    {
        UE_LOG(LogLobby, Warning, TEXT("No available platforms for player %s"), 
               NewPlayer->PlayerState ? *NewPlayer->PlayerState->GetPlayerName() : TEXT("Unknown"));
    }
}

void ALobbyGameMode::Logout(AController* Exiting)
{
    if (ADefaultPlayerState* PlayerState = Cast<ADefaultPlayerState>(Exiting->PlayerState))
    {
        // Remove from GameState roster
        if (LobbyGameState)
        {
            const FString PlayerId = PlayerState->GetUniqueId().ToString();
            LobbyGameState->RemovePlayerFromRoster(PlayerId);
        }

        // Free platform
        if (PlayerState->AssignedPlatform)
        {
            // Find platform index and free it
            for (int32 i = 0; i < LobbyPlatforms.Num(); ++i)
            {
                if (LobbyPlatforms[i] == PlayerState->AssignedPlatform)
                {
                    FreePlatformIndex(i);
                    break;
                }
            }
        }
        
        UE_LOG(LogLobby, Log, TEXT("Player %s left the lobby"), *PlayerState->GetPlayerName());
    }

    Super::Logout(Exiting);
}
{
void ALobbyGameMode::HandleKickRequestedFromPlatform(ALobbyPlatformActor* Platform)
{
    // Find the player on this platform and kick them
    for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(Iterator->Get());
        if (PC)
        {
            ADefaultPlayerState* PlayerState = Cast<ADefaultPlayerState>(PC->PlayerState);
            if (PlayerState && PlayerState->AssignedPlatform == Platform)        
            {
                UE_LOG(LogLobby, Log, TEXT("Kicking player %s from platform"), *PlayerState->GetPlayerName());
                KickPlayer(PC);
                break;
            }
        }
    }
}

void ALobbyGameMode::CheckAllPlayersReady()
{
    // Legacy function - now delegates to GameState for compatibility
    if (LobbyGameState)
    {
        LobbyGameState->EvaluateReadinessAndPhase();
        
        // Maintain backward compatibility: tell host's client to toggle Start button
        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(It->Get());
            if (PC && PC->IsLocalController() && PC->HasAuthority())
            {
                PC->ClientSetStartButtonEnabled(LobbyGameState->bAllPlayersReady);
                break; // only the host needs this
            }
        }
    }
}

void ALobbyGameMode::StartGame()
{
    UE_LOG(LogLobby, Log, TEXT("StartGame called - initiating travel"));
    
    // Set phase to Traveling
    if (LobbyGameState)
    {
        LobbyGameState->SetLobbyPhase(ELobbyPhase::Traveling);
    }

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

void ALobbyGameMode::ServerKickPlayer(const FString& TargetPlayerId, APlayerController* RequestingPlayer)
{
    // Validate requesting player is host
    if (!RequestingPlayer || !RequestingPlayer->PlayerState)
    {
        UE_LOG(LogLobby, Warning, TEXT("Invalid requesting player for kick"));
        return;
    }

    // Check if requesting player is host
    bool bIsHost = false;
    if (LobbyGameState)
    {
        const FString RequesterId = RequestingPlayer->PlayerState->GetUniqueId().ToString();
        FLobbyPlayerRepData* RequesterData = LobbyGameState->FindPlayerInRoster(RequesterId);
        bIsHost = (RequesterData && RequesterData->bIsHost);
    }

    if (!bIsHost)
    {
        UE_LOG(LogLobby, Warning, TEXT("Player %s attempted to kick without host authority"), 
               *RequestingPlayer->PlayerState->GetPlayerName());
        return;
    }

    // Find target player and kick them
    APlayerController* TargetPlayer = FindPlayerControllerById(TargetPlayerId);
    if (TargetPlayer)
    {
        UE_LOG(LogLobby, Log, TEXT("Host %s kicking player %s"), 
               *RequestingPlayer->PlayerState->GetPlayerName(),
               TargetPlayer->PlayerState ? *TargetPlayer->PlayerState->GetPlayerName() : TEXT("Unknown"));
        
        KickPlayer(TargetPlayer);
    }
    else
    {
        UE_LOG(LogLobby, Warning, TEXT("Target player %s not found for kick"), *TargetPlayerId);
    }
}

int32 ALobbyGameMode::FindAvailablePlatformIndex() const
{
    for (int32 i = 0; i < LobbyPlatforms.Num(); ++i)
    {
        if (ALobbyPlatformActor* Platform = Cast<ALobbyPlatformActor>(LobbyPlatforms[i]))
        {
            if (!Platform->OccupyingPawn)
            {
                return i;
            }
        }
    }
    return -1; // No available platforms
}

ALobbyPlatformActor* ALobbyGameMode::GetPlatformByIndex(int32 Index) const
{
    if (Index >= 0 && Index < LobbyPlatforms.Num())
    {
        return Cast<ALobbyPlatformActor>(LobbyPlatforms[Index]);
    }
    return nullptr;
}

void ALobbyGameMode::FreePlatformIndex(int32 PlatformIndex)
{
    if (ALobbyPlatformActor* Platform = GetPlatformByIndex(PlatformIndex))
    {
        if (Platform->OccupyingPawn)
        {
            Platform->OccupyingPawn->Destroy();
            Platform->OccupyingPawn = nullptr;
            UE_LOG(LogLobby, Log, TEXT("Freed platform index %d"), PlatformIndex);
        }
    }
}

APlayerController* ALobbyGameMode::FindPlayerControllerById(const FString& PlayerId) const
{
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC && PC->PlayerState && PC->PlayerState->GetUniqueId().ToString() == PlayerId)
        {
            return PC;
        }
    }
    return nullptr;
}