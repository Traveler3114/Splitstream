#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Engine/Texture2D.h"
#include "NeonRunnerMiniGame.generated.h"

class UNeonRunnerWidget;
class APlayerController;

UENUM(BlueprintType)
enum class EObstacleType : uint8
{
    Spike,
    Tile
};

USTRUCT(BlueprintType)
struct FNeonRunnerObstacle
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Position;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* Texture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Size;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsActive;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EObstacleType Type;
    FNeonRunnerObstacle()
        : Position(FVector2D::ZeroVector), Texture(nullptr), Size(FVector2D(48, 96)), bIsActive(true), Type(EObstacleType::Spike) {
    }
};

USTRUCT(BlueprintType)
struct FNeonRunnerPlayer
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Position;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Velocity;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* Texture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Size;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsOnGround;
    FNeonRunnerPlayer()
        : Position(FVector2D(200.f, 800.f)), Velocity(FVector2D::ZeroVector), Texture(nullptr), Size(FVector2D(96, 96)), bIsOnGround(true) {
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNeonRunnerGameEnded, bool, bWasVictory);

UCLASS(Blueprintable, BlueprintType)
class ECHOESOFTIME_API UNeonRunnerMiniGame : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame") UTexture2D* RunnerTexture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame") UTexture2D* ObstacleTexture_Spike;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame") UTexture2D* TileTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame") TSubclassOf<UNeonRunnerWidget> WidgetClass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame") UInputMappingContext* RunnerIMC;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame") UInputMappingContext* GameplayIMC;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame") UInputAction* JumpAction;

    UPROPERTY(BlueprintAssignable) FNeonRunnerGameEnded OnMiniGameEnded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Gameplay")
    float PlayerMoveSpeed = 720.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Gameplay")
    float JumpVelocity = -1280.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Gameplay")
    float Gravity = 3600.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Gameplay")
    float GroundY = 850.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Tiles")
    float TileSize = 70.f; // One square side in px

    UNeonRunnerMiniGame();

    UFUNCTION(BlueprintCallable, Category = "MiniGame") void StartGame(APlayerController* PlayerController);
    UFUNCTION(BlueprintCallable, Category = "MiniGame") void EndGame();

private:
    FNeonRunnerPlayer Player;
    TArray<FNeonRunnerObstacle> Obstacles;
    TArray<FNeonRunnerObstacle> Tiles;

    float TimeSinceLastObstacle;
    float NextObsDistance;
    float SurvivalTime;
    bool bIsGameOver;
    UPROPERTY() UNeonRunnerWidget* WidgetRef;
    UPROPERTY() APlayerController* OwningController;
    FTimerHandle TickTimerHandle;

    void CreateWidget();
    void SetupInput();
    void CleanupInput();
    void TickGame();
    void SpawnObstacle();
    void UpdatePlayer(float DeltaTime);
    void CheckCollisions();
    void UpdateWidget();
    void GameOver();
    FVector2D GetPlayAreaSize() const;
    FVector2D GetTextureSize(UTexture2D* Texture) const;

    UFUNCTION() void OnJump();

    float PlayerStartX = 320.f;
};