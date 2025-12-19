// FirewallMiniGame.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Engine/Texture2D.h"
#include "FirewallMiniGame.generated.h"

class UFirewallWidget;
class APlayerController;

USTRUCT(BlueprintType)
struct FMiniGamePlayer
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Position;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* Texture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Lives;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MoveSpeed;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Size;
    FMiniGamePlayer() : Position(FVector2D(400.f, 500.f)), Texture(nullptr), Lives(3), MoveSpeed(300.f), Size(FVector2D(50, 50)) {}
};

USTRUCT(BlueprintType)
struct FMiniGameEnemy
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Position;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* Texture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsAlive;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Size;
    FMiniGameEnemy() : Position(FVector2D::ZeroVector), Texture(nullptr), bIsAlive(true), Size(FVector2D(40, 40)) {}
};

USTRUCT(BlueprintType)
struct FMiniGameProjectile
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Position;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* Texture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsPlayerProjectile;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsActive;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Size;
    FMiniGameProjectile() : Position(FVector2D::ZeroVector), Texture(nullptr), bIsPlayerProjectile(true), bIsActive(true), Size(FVector2D(10, 20)) {}
};

UCLASS(Blueprintable, BlueprintType)
class ECHOESOFTIME_API UFirewallMiniGame : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame") UInputMappingContext* FirewallIMC;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame") UInputMappingContext* GameplayIMC;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame") UInputAction* MoveAction;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame") UInputAction* FireAction;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame") TSubclassOf<UFirewallWidget> WidgetClass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame") UTexture2D* PlayerTexture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame") UTexture2D* EnemyTexture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame") UTexture2D* ProjectileTexture;

    UFirewallMiniGame();

    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void StartGame(APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void EndGame();

    // Defers the setup to after widget is ready
    void FinishInitAfterWidgetReady();

    void TryFinishInitWhenCanvasReady();

private:
    FMiniGamePlayer Player;
    TArray<FMiniGameEnemy> Enemies;
    TArray<FMiniGameProjectile> Projectiles;
    int32 Score;
    float PlayerMoveInput = 0.0f; 
    bool bIsGameOver;
    float EnemyMoveDirection;

    UPROPERTY() UFirewallWidget* WidgetRef;
    UPROPERTY() APlayerController* OwningController;
    FTimerHandle TickTimerHandle;

    FVector2D GetPlayAreaSize() const;
    void CreateWidget();
    void SetupInput();
    void CleanupInput();
    void TickGame();
    void SpawnEnemies();
    void UpdatePlayer(float DeltaTime);
    void UpdateEnemies(float DeltaTime);
    void UpdateProjectiles(float DeltaTime);
    void CheckCollisions();
    void UpdateWidget();
    void GameOver();
    void Victory();

    UFUNCTION()
    void OnMoveAxis(const FInputActionValue& Value);
    UFUNCTION()
    void OnMoveAxisCompleted(const FInputActionValue& Value);
    void OnFirePressed();
};