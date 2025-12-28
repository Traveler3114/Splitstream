#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Engine/Texture2D.h"
#include "DinoJumpMiniGame.generated.h"

class UDinoJumpWidget;
class APlayerController;

USTRUCT(BlueprintType)
struct FDinoPlayer
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Position;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Velocity;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* Texture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Size;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsOnGround;
    FDinoPlayer()
        : Position(FVector2D(200.f, 800.f)), Velocity(FVector2D::ZeroVector), Texture(nullptr), Size(FVector2D(96, 96)), bIsOnGround(true) {}
};

USTRUCT(BlueprintType)
struct FDinoObstacle
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Position;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* Texture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Size;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsActive;
    FDinoObstacle()
        : Position(FVector2D::ZeroVector), Texture(nullptr), Size(FVector2D(48, 96)), bIsActive(true) {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDinoGameEnded, bool, bWasVictory);

UCLASS(Blueprintable, BlueprintType)
class ECHOESOFTIME_API UDinoJumpMiniGame : public UObject
{
    GENERATED_BODY()
public:
    // Gameplay assets and mappings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MiniGame") UTexture2D* DinoTexture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MiniGame") UTexture2D* ObstacleTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MiniGame") TSubclassOf<UDinoJumpWidget> WidgetClass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MiniGame") UInputMappingContext* DinoIMC;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MiniGame") UInputMappingContext* GameplayIMC;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MiniGame") UInputAction* JumpAction;

    UPROPERTY(BlueprintAssignable) FDinoGameEnded OnMiniGameEnded;

    // Tunable
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Gameplay")
    float JumpVelocity = -1300.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Gameplay")
    float Gravity = 3600.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Gameplay")
    float GroundY = 850.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Gameplay")
    float DinoStartX = 200.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Gameplay")
    float ObstacleSpeed = 800.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Gameplay")
    float ObstacleMinDistance = 700.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Gameplay")
    float ObstacleMaxDistance = 1200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Gameplay")
    float ObstacleSpawnRangeY = 30.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Gameplay")
    float VictoryTime = 30.f;

    UDinoJumpMiniGame();

    UFUNCTION(BlueprintCallable, Category = "MiniGame") void StartGame(APlayerController* PlayerController);
    UFUNCTION(BlueprintCallable, Category = "MiniGame") void EndGame();

private:
    FDinoPlayer Player;
    TArray<FDinoObstacle> Obstacles;
    float GroundLevelY;
    float TimeSinceLastObstacle;
    float NextObsDistance;
    float SurvivalTime;
    bool bIsGameOver;
    bool bVictoryAchieved;

    UPROPERTY() UDinoJumpWidget* WidgetRef;
    UPROPERTY() APlayerController* OwningController;
    FTimerHandle TickTimerHandle;

    void CreateWidget();
    void SetupInput();
    void CleanupInput();
    void TickGame();
    void SpawnObstacle();
    void UpdatePlayer(float DeltaTime);
    void UpdateObstacles(float DeltaTime);
    void CheckCollisions();
    void UpdateWidget();
    void GameOver();
    void Victory();
    FVector2D GetPlayAreaSize() const;
    FVector2D GetTextureSize(UTexture2D* Texture) const;

    UFUNCTION() void OnJump();
};