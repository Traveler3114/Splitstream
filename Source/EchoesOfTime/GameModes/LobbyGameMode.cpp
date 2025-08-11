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


    PlayerState->OnPlayerReady.AddDynamic(this, &ALobbyGameMode::CheckAllPlayersReady);
    // Find the first available platform
    for (AActor* Actor : LobbyPlatforms)
    {
        ALobbyPlatformActor* Platform = Cast<ALobbyPlatformActor>(Actor);
        if (!Platform->OccupyingPawn)
        {
            APawn* SpawnedPawn = Platform->SpawnCharacterAtPlatform(NewPlayer);
            if (PlayerState)
            {
                PlayerState->AssignedPlatform = Platform;
                Platform->OnKickRequested.AddDynamic(this, &ALobbyGameMode::HandleKickRequestedFromPlatform);
            }
			FGameplayTag DefaultTeamTag = FGameplayTag::RequestGameplayTag(FName("Team.Future"));
			PlayerState->ServerSetTeamTag(DefaultTeamTag); // Set default team tag on PlayerState
            //if (PlayerState->AssignedPlatform)
            //{
            //    PlayerState->AssignedPlatform->TeamTag = DefaultTeamTag;
            //    PlayerState->AssignedPlatform->OnRep_PlayerInfo(); // Update UI immediately
            //}
            break; // Exit after assigning the first available platform
        }
    }
}


void ALobbyGameMode::CheckAllPlayersReady()
{
    bool bEveryoneReady = true;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(It->Get());
        ADefaultPlayerState* PlayerState = Cast<ADefaultPlayerState>(PC->PlayerState);
        if (!PlayerState || !PlayerState->bIsReady)
        {
            bEveryoneReady = false;
            break;
        }
    }

    // Only the server/host should enable the start button
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(It->Get());
        if (PC && PC->IsLocalController() && PC->HasAuthority())
        {
            // Find the LobbyUI instance and enable the start button
            TArray<UUserWidget*> FoundWidgets;
            UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, ULobbyUI::StaticClass(), false);
            for (UUserWidget* Widget : FoundWidgets)
            {
                if (ULobbyUI* LobbyUI = Cast<ULobbyUI>(Widget))
                {
                    LobbyUI->SetStartButtonEnabled(bEveryoneReady);
                }
            }
        }
    }
}

void ALobbyGameMode::StartGame()
{
    // 1. Get all LobbyPlayerControllers
    TArray<AActor*> FoundPlayers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALobbyPlayerController::StaticClass(), FoundPlayers);

    // 2. For each, call OnStartGame
    for (AActor* Actor : FoundPlayers)
    {
        ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(Actor);
        if (PC)
        {
            PC->OnStartGame(); // Assumes this function exists and is BlueprintCallable or implemented in C++
        }
    }

    // 3. Server travel to the map
    FString MapPath = TEXT("/Game/Maps/TestMap?listen");
    GetWorld()->ServerTravel(MapPath);
}


void ALobbyGameMode::HandleKickRequestedFromPlatform(ALobbyPlatformActor* Platform)
{
    if (!Platform) return;

    UWorld* World = GetWorld();
    if (!World) return;

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(It->Get());
        ADefaultPlayerState* PlayerState = Cast<ADefaultPlayerState>(PC->PlayerState);
        if (PC && PlayerState->AssignedPlatform == Platform)
        {
			KickPlayer(PC);
            break;
        }
    }
}