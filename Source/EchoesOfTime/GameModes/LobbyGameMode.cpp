#include "LobbyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/LobbyPlatformActor.h"
#include "Engine/Engine.h" // For GEngine->AddOnScreenDebugMessage

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
        UE_LOG(LogTemp, Warning, TEXT("PostLogin: Refreshed LobbyPlatforms, now found %d"), LobbyPlatforms.Num());
    }
    if (!NewPlayer)
    {
        UE_LOG(LogTemp, Error, TEXT("PostLogin: NewPlayer is nullptr!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("PostLogin: New player joined. LobbyPlatforms.Num() = %d"), LobbyPlatforms.Num());

    // Find the first available platform
    for (AActor* Actor : LobbyPlatforms)
    {
        ALobbyPlatformActor* Platform = Cast<ALobbyPlatformActor>(Actor);
        if (!Platform)
        {
            UE_LOG(LogTemp, Error, TEXT("PostLogin: Actor in LobbyPlatforms is not a LobbyPlatformActor!"));
            continue;
        }

        UE_LOG(LogTemp, Warning, TEXT("PostLogin: Checking platform %s, OccupyingPawn=%s"),
            *Platform->GetName(),
            Platform->OccupyingPawn ? *Platform->OccupyingPawn->GetName() : TEXT("nullptr"));

        if (!Platform->OccupyingPawn)
        {
            APawn* SpawnedPawn = Platform->SpawnCharacterAtPlatform(NewPlayer);
            UE_LOG(LogTemp, Warning, TEXT("PostLogin: SpawnedPawn=%s"), SpawnedPawn ? *SpawnedPawn->GetName() : TEXT("nullptr"));
            break; // Exit after assigning the first available platform
        }
    }
}