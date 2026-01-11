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
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 HP;
    FMiniGameEnemy() : Position(FVector2D::ZeroVector), Texture(nullptr), bIsAlive(true), Size(FVector2D(40, 40)), HP(1) {}
};

USTRUCT(BlueprintType)
struct FMiniGameHeavyEnemy
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Position;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* Texture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsAlive;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Size;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 HP;
    FMiniGameHeavyEnemy() : Position(FVector2D::ZeroVector), Texture(nullptr), bIsAlive(true), Size(FVector2D(56, 80)), HP(3) {}
};

USTRUCT(BlueprintType)
struct FMiniGameProjectile
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Position;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* Texture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsActive;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Size;
    FMiniGameProjectile() : Position(FVector2D::ZeroVector), Texture(nullptr), bIsActive(true), Size(FVector2D(10, 20)) {}
};

USTRUCT(BlueprintType)
struct FMiniGameEnemyBullet
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Position;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* Texture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsActive;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Size;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Velocity;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LifeTime;
    FMiniGameEnemyBullet() : Position(FVector2D::ZeroVector), Texture(nullptr), bIsActive(true), Size(FVector2D(10, 25)), Velocity(FVector2D::ZeroVector), LifeTime(0.0f) {}
};

USTRUCT(BlueprintType)
struct FMiniGameHeavyEnemyBullet
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Position;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* Texture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsActive;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Size;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Velocity;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LifeTime;
    FMiniGameHeavyEnemyBullet() : Position(FVector2D::ZeroVector), Texture(nullptr), bIsActive(true), Size(FVector2D(24, 60)), Velocity(FVector2D::ZeroVector), LifeTime(0.0f) {}
};

USTRUCT(BlueprintType)
struct FMiniGameBoss
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Position;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Size;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* Texture;
    FMiniGameBoss() : Position(FVector2D::ZeroVector), Size(FVector2D(96, 96)), Texture(nullptr) {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFirewallMiniGameEnded, bool, bWasVictory);

UCLASS(Blueprintable, BlueprintType)
class SPLITSTREAM_API UFirewallMiniGame : public UObject
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame") UTexture2D* HeavyEnemyTexture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame") UTexture2D* ProjectileTexture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame") UTexture2D* EnemyBulletTexture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame") UTexture2D* HeavyEnemyBulletTexture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame") UTexture2D* BossTexture;


    // In FirewallMiniGame.h

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|EasterEgg") UTexture2D* EasterEggPlayerTexture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|EasterEgg") UTexture2D* EasterEggEnemyTexture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|EasterEgg") UTexture2D* EasterEggHeavyEnemyTexture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|EasterEgg") UTexture2D* EasterEggProjectileTexture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|EasterEgg") UTexture2D* EasterEggEnemyBulletTexture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|EasterEgg") UTexture2D* EasterEggHeavyEnemyBulletTexture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|EasterEgg") UTexture2D* EasterEggBossTexture;

    UPROPERTY(BlueprintAssignable)
    FFirewallMiniGameEnded OnMiniGameEnded;

    // ==== TUNABLE GAMEPLAY VARIABLES ====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Boss Tuning")
    int32 BossTotalHP = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Boss Tuning")
    int32 BossLowHPThreshold = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Boss Tuning")
    float BossModeDuration = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Boss Tuning")
    float BossBounceSpeed = 370.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Boss Tuning")
    float BossBulletInterval = 0.40f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Boss Tuning")
    int32 BossBulletCountNormal = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Boss Tuning")
    int32 BossBulletCountHeavy = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Projectile Tuning")
    float EnemyBulletLifespan = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Projectile Tuning")
    float HeavyEnemyBulletLifespan = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Enemy Tuning")
    float FixedEnemySpawnInterval = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Enemy Tuning")
    float FixedEnemyFireInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Enemy Tuning")
    float FixedEnemyFallSpeed = 0.20f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Enemy Tuning")
    float FixedHeavyEnemyFallSpeed = 0.17f;
    // =====================================

    UFirewallMiniGame();

    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void StartGame(APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void EndGame();

    void FinishInitAfterWidgetReady();
    void TryFinishInitWhenCanvasReady();
    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void OnDifficultyIncrease(int32 NewDifficulty);
    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void OnBossFightStart();

    UPROPERTY(BlueprintReadWrite, Category = "EasterEgg")
    bool bUseEasterEggSprites = false;

private:
    FMiniGamePlayer Player;
    TArray<FMiniGameEnemy> Enemies;
    TArray<FMiniGameHeavyEnemy> HeavyEnemies;
    TArray<FMiniGameProjectile> Projectiles;
    TArray<FMiniGameEnemyBullet> EnemyBullets;
    TArray<FMiniGameHeavyEnemyBullet> HeavyEnemyBullets;

    FVector2D PlayerMoveInput = FVector2D::ZeroVector;
    bool bIsGameOver;

    UPROPERTY() UFirewallWidget* WidgetRef;
    UPROPERTY() APlayerController* OwningController;
    FTimerHandle TickTimerHandle;

    // Spawning/speed
    float EnemySpawnInterval;
    float TimeSinceLastEnemySpawn;
    float EnemyFireInterval;
    float TimeSinceLastEnemyFire;
    float EnemyFallSpeed;      
    float HeavyEnemyFallSpeed; 

    // Timed survival, difficulty, boss
    float GameElapsedTime = 0.0f;
    int32 DifficultyLevel = 0;
    bool bDidBossStart = false;
    bool bPendingBoss = false;

    float Initial_EnemySpawnInterval;
    float Initial_EnemyFireInterval;
    float Initial_EnemyFallSpeed;
    float Initial_HeavyEnemyFallSpeed;

    // Boss
    FMiniGameBoss Boss;
    bool bIsBossActive;
    int32 BossHP;
    int32 BossCurrentMode;   
    float BossTimeInMode;
    float TimeSinceBossBullet;
    FVector2D BossVelocity;

    FVector2D GetPlayAreaSize() const;
    FVector2D GetTextureSize(UTexture2D* Texture) const;
    void CreateWidget();
    void SetupInput();
    void CleanupInput();
    void TickGame();
    void SpawnEnemy();
    void SpawnHeavyEnemy();
    void SpawnPlayerBullet();
    void SpawnEnemyBullet(const FVector2D& EnemyPosition, UTexture2D* BulletTex, FVector2D ExtraVelocity = FVector2D::ZeroVector);
    void SpawnHeavyEnemyBullet(const FVector2D& EnemyPosition, UTexture2D* BulletTex, FVector2D ExtraVelocity = FVector2D::ZeroVector);
    void UpdatePlayer(float DeltaTime);
    void UpdateEnemies(float DeltaTime);
    void UpdateHeavyEnemies(float DeltaTime);
    void UpdateProjectiles(float DeltaTime);
    void UpdateEnemyBullets(float DeltaTime);
    void UpdateHeavyEnemyBullets(float DeltaTime);
    void CheckCollisions();
    void UpdateWidget();
    void GameOver();
    void Victory();

    void UpdateBoss(float DeltaTime);

    UFUNCTION()
    void OnMoveAxis(const FInputActionValue& Value);
    UFUNCTION()
    void OnMoveAxisCompleted(const FInputActionValue& Value);
    void OnFirePressed();
};