#include "FirewallMiniGame.h"
#include "Widgets/Minigames/FirewallWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/World.h"

#define BOSS_TOTAL_HP          100
#define BOSS_LOWHP_THRESHOLD   10
#define BOSS_MODE_DURATION     5.0f
#define BOSS_BOUNCE_SPEED      370.f
#define BOSS_BULLET_INTERVAL   0.40f
#define BOSS_BULLET_COUNT_MIN  3
#define BOSS_BULLET_COUNT_MAX  5

// Lifespan for enemy and heavy enemy bullets (in seconds)
#define ENEMY_BULLET_LIFESPAN       5.0f
#define HEAVY_ENEMY_BULLET_LIFESPAN 5.0f

UFirewallMiniGame::UFirewallMiniGame()
    : bIsGameOver(false)
    , EnemySpawnInterval(0.8f)
    , TimeSinceLastEnemySpawn(0.0f)
    , EnemyFireInterval(1.0f)
    , TimeSinceLastEnemyFire(0.0f)
    , EnemyFallSpeed(0.20f)
    , HeavyEnemyFallSpeed(0.17f)
    , WidgetRef(nullptr)
    , OwningController(nullptr)
    , GameElapsedTime(0.0f)
    , DifficultyLevel(0)
    , bDidBossStart(false)
    , bPendingBoss(false)
    , Initial_EnemySpawnInterval(0.8f)
    , Initial_EnemyFireInterval(1.0f)
    , Initial_EnemyFallSpeed(0.20f)
    , Initial_HeavyEnemyFallSpeed(0.17f)
    // Boss related
    , bIsBossActive(false)
    , BossHP(0)
    , BossCurrentMode(0)
    , BossTimeInMode(0.0f)
    , TimeSinceBossBullet(0.0f)
{}

FVector2D UFirewallMiniGame::GetPlayAreaSize() const
{
    if (WidgetRef && WidgetRef->GameCanvas)
        return WidgetRef->GameCanvas->GetCachedGeometry().GetLocalSize();
    return FVector2D(1920, 1080);
}

FVector2D UFirewallMiniGame::GetTextureSize(UTexture2D* Texture) const
{
    if (Texture)
        return FVector2D(Texture->GetImportedSize().X, Texture->GetImportedSize().Y);
    return FVector2D(96, 96);
}

void UFirewallMiniGame::StartGame(APlayerController* PlayerController)
{
    if (!PlayerController)
        return;

    OwningController = PlayerController;
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwningController->GetLocalPlayer()))
    {
        if (GameplayIMC) Subsystem->RemoveMappingContext(GameplayIMC);
        if (FirewallIMC) Subsystem->AddMappingContext(FirewallIMC, 100);
    }

    bIsGameOver = false;
    TimeSinceLastEnemySpawn = 0.0f;
    TimeSinceLastEnemyFire = 0.0f;
    GameElapsedTime = 0.0f;
    DifficultyLevel = 0;
    bDidBossStart = false;
    bPendingBoss = false;

    EnemySpawnInterval   = Initial_EnemySpawnInterval;
    EnemyFireInterval    = Initial_EnemyFireInterval;
    EnemyFallSpeed       = Initial_EnemyFallSpeed;
    HeavyEnemyFallSpeed  = Initial_HeavyEnemyFallSpeed;

    // Boss
    bIsBossActive = false;
    BossHP = 0;
    BossCurrentMode = 0;
    BossTimeInMode = 0.0f;
    TimeSinceBossBullet = 0.0f;
    BossVelocity = FVector2D::ZeroVector;

    CreateWidget();
    if (UWorld* World = OwningController->GetWorld())
        World->GetTimerManager().SetTimerForNextTick(this, &UFirewallMiniGame::TryFinishInitWhenCanvasReady);
}

void UFirewallMiniGame::TryFinishInitWhenCanvasReady()
{
    bool bReady = false;
    if (WidgetRef && WidgetRef->GameCanvas)
    {
        FVector2D Area = WidgetRef->GameCanvas->GetCachedGeometry().GetLocalSize();
        bReady = (Area.X > 0.0f && Area.Y > 0.0f);
    }
    if (bReady)
    {
        FinishInitAfterWidgetReady();
    }
    else
    {
        if (OwningController)
        {
            if (UWorld* World = OwningController->GetWorld())
                World->GetTimerManager().SetTimerForNextTick(this, &UFirewallMiniGame::TryFinishInitWhenCanvasReady);
        }
    }
}

void UFirewallMiniGame::FinishInitAfterWidgetReady()
{
    FVector2D Area = GetPlayAreaSize();
    Player = FMiniGamePlayer();
    Player.Position = FVector2D(Area.X * 0.5f, Area.Y - Area.Y * 0.12f);
    Player.Texture = PlayerTexture;
    Player.Size = GetTextureSize(PlayerTexture);
    Player.MoveSpeed = 600.f; // SPEED UP SPACESHIP!

    Enemies.Empty();
    HeavyEnemies.Empty();
    Projectiles.Empty();
    EnemyBullets.Empty();
    HeavyEnemyBullets.Empty();

    // Boss
    bIsBossActive = false;
    BossHP = 0;
    BossCurrentMode = 0;
    BossTimeInMode = 0.0f;
    TimeSinceBossBullet = 0.0f;
    BossVelocity = FVector2D::ZeroVector;

    SetupInput();

    if (UWorld* World = OwningController->GetWorld())
        World->GetTimerManager().SetTimer(
            TickTimerHandle,
            this,
            &UFirewallMiniGame::TickGame,
            0.016f,
            true
        );

    UpdateWidget();
}

void UFirewallMiniGame::SpawnEnemy()
{
    if (bIsBossActive) return; // Don't spawn regulars in boss
    FVector2D Area = GetPlayAreaSize();
    float MinX = Area.X * 0.06f;
    float MaxX = Area.X * 0.94f;
    float RandX = FMath::RandRange(MinX, MaxX);

    float t = GameElapsedTime;
    float heavySpawnChance = 0.f;
    if (t < 20.0f) {
        heavySpawnChance = 0.0f;
    } else if (t < 40.0f) {
        heavySpawnChance = 0.5f;
    } else {
        heavySpawnChance = 1.0f;
    }

    if (FMath::FRand() < heavySpawnChance)
    {
        SpawnHeavyEnemy();
    }
    else
    {
        FMiniGameEnemy Enemy;
        Enemy.Position = FVector2D(RandX, 0.f + 32.0f);
        Enemy.Texture = EnemyTexture;
        Enemy.Size = GetTextureSize(EnemyTexture);
        Enemy.HP = 1;
        Enemy.bIsAlive = true;
        Enemies.Add(Enemy);
    }
}

void UFirewallMiniGame::SpawnHeavyEnemy()
{
    if (bIsBossActive) return;
    FVector2D Area = GetPlayAreaSize();
    float MinX = Area.X * 0.06f;
    float MaxX = Area.X * 0.94f;
    float RandX = FMath::RandRange(MinX, MaxX);

    FMiniGameHeavyEnemy HeavyEnemy;
    HeavyEnemy.Position = FVector2D(RandX, 0.f + 32.0f);
    HeavyEnemy.Texture = HeavyEnemyTexture;
    HeavyEnemy.Size = GetTextureSize(HeavyEnemyTexture);
    HeavyEnemy.HP = 3;
    HeavyEnemy.bIsAlive = true;
    HeavyEnemies.Add(HeavyEnemy);
}

void UFirewallMiniGame::SpawnPlayerBullet()
{
    FMiniGameProjectile Projectile;
    Projectile.Position = Player.Position;
    Projectile.Texture = ProjectileTexture;
    Projectile.Size = GetTextureSize(ProjectileTexture);
    Projectile.bIsActive = true;
    Projectiles.Add(Projectile);
}

// Overloaded with explicit velocity for bounce logic
void UFirewallMiniGame::SpawnEnemyBullet(const FVector2D& EnemyPosition, UTexture2D* BulletTex, FVector2D ExtraVelocity)
{
    FMiniGameEnemyBullet Bullet;
    Bullet.Position = EnemyPosition;
    Bullet.Texture = BulletTex;
    Bullet.Size = GetTextureSize(BulletTex);
    Bullet.bIsActive = true;
    // Set random velocity for bounce even for regular enemies
    if (ExtraVelocity.IsNearlyZero())
    {
        // straight downward plus small random X
        float angle = FMath::FRandRange(-PI / 6.f, PI / 6.f); // -30 to 30 degrees
        float speed = GetPlayAreaSize().Y * 0.38f; // gives similar speed to original code downward
        Bullet.Velocity = FVector2D(FMath::Sin(angle) * speed, FMath::Cos(angle) * speed);
    }
    else
    {
        Bullet.Velocity = ExtraVelocity;
    }
    // Add a lifespan field for manual timeout, using BlueprintReadWrite
    Bullet.LifeTime = 0.0f;
    EnemyBullets.Add(Bullet);
}

void UFirewallMiniGame::SpawnHeavyEnemyBullet(const FVector2D& EnemyPosition, UTexture2D* BulletTex, FVector2D ExtraVelocity)
{
    FMiniGameHeavyEnemyBullet Bullet;
    Bullet.Position = EnemyPosition;
    Bullet.Texture = BulletTex;
    Bullet.Size = GetTextureSize(BulletTex);
    Bullet.bIsActive = true;
    if (ExtraVelocity.IsNearlyZero())
    {
        float angle = FMath::FRandRange(-PI / 4.f, PI / 4.f); // -45 to 45 degrees
        float speed = GetPlayAreaSize().Y * 0.56f; // gives similar speed as before
        Bullet.Velocity = FVector2D(FMath::Sin(angle) * speed, FMath::Cos(angle) * speed);
    }
    else
    {
        Bullet.Velocity = ExtraVelocity;
    }
    Bullet.LifeTime = 0.0f;
    HeavyEnemyBullets.Add(Bullet);
}

void UFirewallMiniGame::UpdateEnemies(float DeltaTime)
{
    if (bIsBossActive) return; // No regulars in boss fight
    FVector2D Area = GetPlayAreaSize();
    float FallSpeed = Area.Y * EnemyFallSpeed;
    for (int32 i = 0; i < Enemies.Num(); ++i)
    {
        FMiniGameEnemy& Enemy = Enemies[i];
        if (!Enemy.bIsAlive) continue;
        Enemy.Position.Y += FallSpeed * DeltaTime;
        if (Enemy.Position.Y - Enemy.Size.Y * 0.5f > Area.Y)
            Enemy.bIsAlive = false;
    }
    Enemies.RemoveAll([](const FMiniGameEnemy& Enemy) { return !Enemy.bIsAlive; });
}

void UFirewallMiniGame::UpdateHeavyEnemies(float DeltaTime)
{
    if (bIsBossActive) return;
    FVector2D Area = GetPlayAreaSize();
    float FallSpeed = Area.Y * HeavyEnemyFallSpeed;
    for (int32 i = 0; i < HeavyEnemies.Num(); ++i)
    {
        FMiniGameHeavyEnemy& Enemy = HeavyEnemies[i];
        if (!Enemy.bIsAlive) continue;
        Enemy.Position.Y += FallSpeed * DeltaTime;
        if (Enemy.Position.Y - Enemy.Size.Y * 0.5f > Area.Y)
            Enemy.bIsAlive = false;
    }
    HeavyEnemies.RemoveAll([](const FMiniGameHeavyEnemy& Enemy) { return !Enemy.bIsAlive; });
}

void UFirewallMiniGame::OnFirePressed()
{
    if (bIsGameOver)
        return;
    SpawnPlayerBullet();
}

void UFirewallMiniGame::UpdateProjectiles(float DeltaTime)
{
    FVector2D Area = GetPlayAreaSize();
    float ProjectileSpeed = Area.Y * 0.7f;
    for (int32 i = 0; i < Projectiles.Num(); ++i)
    {
        FMiniGameProjectile& Projectile = Projectiles[i];
        if (!Projectile.bIsActive) continue;
        Projectile.Position.Y -= ProjectileSpeed * DeltaTime;
        if (Projectile.Position.Y < 0.0f || Projectile.Position.Y > Area.Y)
            Projectile.bIsActive = false;
    }
    Projectiles.RemoveAll([](const FMiniGameProjectile& P) { return !P.bIsActive; });
}

// Bullet bouncing and timeout enabled ALWAYS

void UFirewallMiniGame::UpdateEnemyBullets(float DeltaTime)
{
    FVector2D Area = GetPlayAreaSize();
    float BulletSpeed = Area.Y * 0.38f;
    for (int32 i = 0; i < EnemyBullets.Num(); ++i)
    {
        FMiniGameEnemyBullet& Bull = EnemyBullets[i];
        if (!Bull.bIsActive) continue;

        // Always bounce logic, not just boss. Ignore .Velocity == 0.
        Bull.Position += Bull.Velocity * DeltaTime;

        // Bounce bullets off all 4 sides
        if (Bull.Position.X < Bull.Size.X*0.5f && Bull.Velocity.X < 0)  Bull.Velocity.X *= -1.0f;
        if (Bull.Position.X > Area.X-Bull.Size.X*0.5f && Bull.Velocity.X > 0) Bull.Velocity.X *= -1.0f;
        if (Bull.Position.Y < Bull.Size.Y*0.5f && Bull.Velocity.Y < 0)  Bull.Velocity.Y *= -1.0f;
        if (Bull.Position.Y > Area.Y-Bull.Size.Y*0.5f && Bull.Velocity.Y > 0) Bull.Velocity.Y *= -1.0f;

        // Increase lifetime and check max
        Bull.LifeTime += DeltaTime;
        // Remove if out of life
        if (Bull.LifeTime > ENEMY_BULLET_LIFESPAN)
            Bull.bIsActive = false;
    }
    EnemyBullets.RemoveAll([](const FMiniGameEnemyBullet& B) { return !B.bIsActive; });
}

void UFirewallMiniGame::UpdateHeavyEnemyBullets(float DeltaTime)
{
    FVector2D Area = GetPlayAreaSize();
    float BulletSpeed = Area.Y * 0.56f;
    for (int32 i = 0; i < HeavyEnemyBullets.Num(); ++i)
    {
        FMiniGameHeavyEnemyBullet& Bull = HeavyEnemyBullets[i];
        if (!Bull.bIsActive) continue;

        // Always bounce logic for heavy bullets too, always
        Bull.Position += Bull.Velocity * DeltaTime;
        if (Bull.Position.X < Bull.Size.X*0.5f && Bull.Velocity.X < 0) Bull.Velocity.X *= -1.0f;
        if (Bull.Position.X > Area.X - Bull.Size.X*0.5f && Bull.Velocity.X > 0) Bull.Velocity.X *= -1.0f;
        if (Bull.Position.Y < Bull.Size.Y*0.5f && Bull.Velocity.Y < 0) Bull.Velocity.Y *= -1.0f;
        if (Bull.Position.Y > Area.Y - Bull.Size.Y*0.5f && Bull.Velocity.Y > 0) Bull.Velocity.Y *= -1.0f;

        // Increase lifetime and check max
        Bull.LifeTime += DeltaTime;
        if (Bull.LifeTime > HEAVY_ENEMY_BULLET_LIFESPAN)
            Bull.bIsActive = false;
    }
    HeavyEnemyBullets.RemoveAll([](const FMiniGameHeavyEnemyBullet& B) { return !B.bIsActive; });
}

void UFirewallMiniGame::CheckCollisions()
{
    FVector2D Area = GetPlayAreaSize();
    float PlayerCollisionRadius = FMath::Max(Player.Size.X, Player.Size.Y) * 0.5f;
    bool bVictory = false;
    bool bDefeat = false;

    // Projectiles vs Boss and Enemies
    for (FMiniGameProjectile& Projectile : Projectiles)
    {
        if (!Projectile.bIsActive) continue;

        // --- Hit boss ---
        if (bIsBossActive && BossHP > 0)
        {
            float Dist = FVector2D::Distance(Projectile.Position, Boss.Position);
            float CollideR = FMath::Max(Boss.Size.X, Boss.Size.Y) * 0.49f;
            if (Dist < CollideR)
            {
                Projectile.bIsActive = false;
                BossHP--;
                if (BossHP <= 0)
                    bVictory = true;
                continue; // skip to next projectile
            }
        }

        // --- Hit enemies ---
        if (!bIsBossActive) {
            for (FMiniGameEnemy& Enemy : Enemies)
            {
                if (!Enemy.bIsAlive) continue;
                float Distance = FVector2D::Distance(Projectile.Position, Enemy.Position);
                float CollisionRadius = FMath::Max(Enemy.Size.X, Enemy.Size.Y) * 0.42f;
                if (Distance < CollisionRadius)
                {
                    Enemy.HP--;
                    Projectile.bIsActive = false;
                    if (Enemy.HP <= 0)
                        Enemy.bIsAlive = false;
                    break;
                }
            }
            for (FMiniGameHeavyEnemy& Enemy : HeavyEnemies)
            {
                if (!Enemy.bIsAlive) continue;
                float Distance = FVector2D::Distance(Projectile.Position, Enemy.Position);
                float CollisionRadius = FMath::Max(Enemy.Size.X, Enemy.Size.Y) * 0.46f;
                if (Distance < CollisionRadius)
                {
                    Enemy.HP--;
                    Projectile.bIsActive = false;
                    if (Enemy.HP <= 0)
                        Enemy.bIsAlive = false;
                    break;
                }
            }
        }
    }

    // ***** NEW: Player bullet vs Enemy Bullets (and Heavy Enemy Bullets) *****
    for (FMiniGameProjectile& Projectile : Projectiles)
    {
        if (!Projectile.bIsActive) continue;

        // --- Against regular enemy bullets ---
        for (FMiniGameEnemyBullet& Bull : EnemyBullets)
        {
            if (!Bull.bIsActive) continue;
            float Distance = FVector2D::Distance(Projectile.Position, Bull.Position);
            float CollisionRadius = FMath::Max(Projectile.Size.X, Projectile.Size.Y) * 0.5f +
                                   FMath::Max(Bull.Size.X, Bull.Size.Y) * 0.5f;
            if (Distance < CollisionRadius)
            {
                Projectile.bIsActive = false;
                Bull.bIsActive = false;
                break; // This projectile is destroyed so no further checks
            }
        }
        if (!Projectile.bIsActive) continue;

        // --- Against heavy enemy bullets ---
        for (FMiniGameHeavyEnemyBullet& HBull : HeavyEnemyBullets)
        {
            if (!HBull.bIsActive) continue;
            float Distance = FVector2D::Distance(Projectile.Position, HBull.Position);
            float CollisionRadius = FMath::Max(Projectile.Size.X, Projectile.Size.Y) * 0.5f +
                                   FMath::Max(HBull.Size.X, HBull.Size.Y) * 0.5f;
            if (Distance < CollisionRadius)
            {
                Projectile.bIsActive = false;
                HBull.bIsActive = false;
                break;
            }
        }
    }
    // ***** END NEW *****

    // Enemy Bullets vs Player
    for (FMiniGameEnemyBullet& Bull : EnemyBullets)
    {
        if (!Bull.bIsActive) continue;
        float Distance = FVector2D::Distance(Bull.Position, Player.Position);
        if (Distance < PlayerCollisionRadius)
        {
            Bull.bIsActive = false;
            Player.Lives -= 1;
            if (Player.Lives <= 0)
                bDefeat = true;
        }
    }

    for (FMiniGameHeavyEnemyBullet& Bull : HeavyEnemyBullets)
    {
        if (!Bull.bIsActive) continue;
        float Distance = FVector2D::Distance(Bull.Position, Player.Position);
        if (Distance < PlayerCollisionRadius)
        {
            Bull.bIsActive = false;
            Player.Lives = 0;
            bDefeat = true;
        }
    }

    // Now cleanup projectiles/enemies/bullets after all looping is done:
    Projectiles.RemoveAll([](const FMiniGameProjectile& P) { return !P.bIsActive; });
    Enemies.RemoveAll([](const FMiniGameEnemy& Enemy) { return !Enemy.bIsAlive; });
    HeavyEnemies.RemoveAll([](const FMiniGameHeavyEnemy& Enemy) { return !Enemy.bIsAlive; });
    EnemyBullets.RemoveAll([](const FMiniGameEnemyBullet& B) { return !B.bIsActive; });
    HeavyEnemyBullets.RemoveAll([](const FMiniGameHeavyEnemyBullet& B) { return !B.bIsActive; });

    // End game if needed, but only after all unsafe array usage is over!
    if (bVictory)
        Victory();
    if (bDefeat)
        GameOver();
}
void UFirewallMiniGame::EndGame()
{
    if (OwningController)
    {
        if (UWorld* World = OwningController->GetWorld())
            World->GetTimerManager().ClearTimer(TickTimerHandle);

        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwningController->GetLocalPlayer()))
        {
            if (FirewallIMC)
                Subsystem->RemoveMappingContext(FirewallIMC);
            if (GameplayIMC)
                Subsystem->AddMappingContext(GameplayIMC, 0);
        }
    }
    if (WidgetRef)
    {
        WidgetRef->RemoveFromParent();
        WidgetRef = nullptr;
    }
    bIsGameOver = true;
    CleanupInput();
    Enemies.Empty();
    HeavyEnemies.Empty();
    Projectiles.Empty();
    EnemyBullets.Empty();
    HeavyEnemyBullets.Empty();
    OwningController = nullptr;
}

void UFirewallMiniGame::CreateWidget()
{
    if (!OwningController || !WidgetClass)
        return;
    WidgetRef = ::CreateWidget<UFirewallWidget>(OwningController, WidgetClass);
    if (WidgetRef)
        WidgetRef->AddToViewport(0);
}

void UFirewallMiniGame::SetupInput()
{
    if (!OwningController)
        return;
    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(OwningController->InputComponent);
    if (EnhancedInput)
    {
        EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &UFirewallMiniGame::OnMoveAxis);
        EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &UFirewallMiniGame::OnMoveAxisCompleted);
        EnhancedInput->BindAction(FireAction, ETriggerEvent::Started, this, &UFirewallMiniGame::OnFirePressed);
    }
}


void UFirewallMiniGame::CleanupInput()
{
    // Stub for input cleanup
}

// --------------------------
// Boss Implementation
// --------------------------
void UFirewallMiniGame::OnBossFightStart()
{
    // Called automatically when 60s up & all enemies dead!
    bIsBossActive = true;
    BossHP = BOSS_TOTAL_HP;
    FVector2D Area = GetPlayAreaSize();
    Boss.Position = FVector2D(Area.X/2.f, Area.Y*0.22f);
    Boss.Size = GetTextureSize(BossTexture ? BossTexture : HeavyEnemyTexture);
    Boss.Texture = (BossTexture != nullptr) ? BossTexture : HeavyEnemyTexture;
    // Start in mode 0
    BossCurrentMode = 0;
    BossTimeInMode = 0.0f;
    TimeSinceBossBullet = 0.0f;
    BossVelocity = FVector2D(BOSS_BOUNCE_SPEED, BOSS_BOUNCE_SPEED); // Diagonal to start
}

void UFirewallMiniGame::UpdateBoss(float DeltaTime)
{
    if (!bIsBossActive || BossHP <= 0) return;

    FVector2D Area = GetPlayAreaSize();
    BossTimeInMode += DeltaTime;
    TimeSinceBossBullet += DeltaTime;

    // Determine mode
    int NumColumns = FMath::RandRange(BOSS_BULLET_COUNT_MIN, BOSS_BULLET_COUNT_MAX);

    // If low HP, it's mode 2
    int RealMode = BossCurrentMode;
    if (BossHP <= BOSS_LOWHP_THRESHOLD) RealMode = 2;

    if (RealMode == 0)
    {
        // Bounce anywhere (full screen, x and y)
        Boss.Position += BossVelocity * DeltaTime;
        // Bounce X
        if ((Boss.Position.X < Boss.Size.X*0.5f && BossVelocity.X<0) ||
            (Boss.Position.X > Area.X-Boss.Size.X*0.5f && BossVelocity.X>0)) BossVelocity.X *= -1.0f;
        // Bounce Y
        if ((Boss.Position.Y < Boss.Size.Y*0.5f && BossVelocity.Y<0) ||
            (Boss.Position.Y > Area.Y-Boss.Size.Y*0.5f && BossVelocity.Y>0)) BossVelocity.Y *= -1.0f;

        // Fire: Normal bullets (bounce) except if lowHP mode—then heavy!
        if (TimeSinceBossBullet >= BOSS_BULLET_INTERVAL)
        {
            int NShots = 2 + (BossHP<=BOSS_LOWHP_THRESHOLD ? 5 : FMath::RandRange(2,4));
            for (int i=0; i<NShots; ++i)
            {
                float ang = FMath::FRandRange(0, 2.f*PI);
                FVector2D Vel = FVector2D(FMath::Cos(ang), FMath::Sin(ang)).GetSafeNormal() * (BOSS_BOUNCE_SPEED+110.f);

                if(BossHP > BOSS_LOWHP_THRESHOLD)
                    SpawnEnemyBullet(Boss.Position, EnemyBulletTexture, Vel);
                else
                    SpawnHeavyEnemyBullet(Boss.Position, HeavyEnemyBulletTexture, Vel);
            }
            TimeSinceBossBullet = 0.0f;
        }
    }
    else // RealMode == 1
    {
        // Bounce on top half, left/right only
        Boss.Position += FVector2D(BossVelocity.X, 0.f) * DeltaTime;
        float minY = Boss.Size.Y*0.5f;
        float midY = Area.Y*0.19f;
        float maxY = Area.Y*0.38f;

        Boss.Position.Y = FMath::Clamp(Boss.Position.Y, minY, maxY);

        if ((Boss.Position.X < Boss.Size.X*0.5f && BossVelocity.X<0) ||
            (Boss.Position.X > Area.X-Boss.Size.X*0.5f && BossVelocity.X>0)) BossVelocity.X *= -1.0f;

        // Fire: Heavy bullets, spread shot columns
        if (TimeSinceBossBullet >= BOSS_BULLET_INTERVAL + 0.16f)
        {
            int Columns = FMath::RandRange(3, 5);
            float left = Boss.Size.X*0.5f + 10.f;
            float right = Area.X - Boss.Size.X*0.5f - 10.f;
            for (int i=0; i<Columns; ++i)
            {
                float frac = Columns==1 ? 0.5f : float(i)/(Columns-1);
                float x = FMath::Lerp(left, right, frac);
                FVector2D BulletPos(x, Boss.Position.Y+Boss.Size.Y*0.5f+6.0f);
                FVector2D Vel = FVector2D(0.f, 380.f+FMath::FRandRange(-65.f,65.f));
                SpawnHeavyEnemyBullet(BulletPos, HeavyEnemyBulletTexture, Vel);
            }
            TimeSinceBossBullet = 0.0f;
        }
    }

    // Switch mode?
    if (BossHP > BOSS_LOWHP_THRESHOLD && BossTimeInMode > BOSS_MODE_DURATION)
    {
        BossCurrentMode = (BossCurrentMode + 1) % 2;
        BossTimeInMode = 0.0f;
        if (BossCurrentMode == 0)
        {
            BossVelocity = FVector2D(BOSS_BOUNCE_SPEED, BOSS_BOUNCE_SPEED);
        }
        else
        {
            float sideVel = FMath::RandBool() ? BOSS_BOUNCE_SPEED : -BOSS_BOUNCE_SPEED;
            BossVelocity = FVector2D(sideVel, 0.0f);
        }
    }
}

void UFirewallMiniGame::TickGame()
{
    if (bIsGameOver) return;
    float DeltaTime = 0.016f;
    GameElapsedTime += DeltaTime;

    // (Normal phase) Stop at boss
    if (!bIsBossActive)
    {
        if (DifficultyLevel == 0 && GameElapsedTime >= 20.0f)
        {
            DifficultyLevel = 1;
            EnemySpawnInterval = 0.55f;
            EnemyFireInterval  = 0.75f;
            EnemyFallSpeed     = 0.23f;
            HeavyEnemyFallSpeed= 0.20f;
            OnDifficultyIncrease(DifficultyLevel);
        }
        else if (DifficultyLevel == 1 && GameElapsedTime >= 40.0f)
        {
            DifficultyLevel = 2;
            EnemySpawnInterval = 0.36f;
            EnemyFireInterval  = 0.45f;
            EnemyFallSpeed     = 0.24f;
            HeavyEnemyFallSpeed= 0.38f;
            OnDifficultyIncrease(DifficultyLevel);
        }

        if (!bPendingBoss && !bDidBossStart)
        {
            TimeSinceLastEnemySpawn += DeltaTime;
            if (TimeSinceLastEnemySpawn >= EnemySpawnInterval)
            {
                SpawnEnemy();
                TimeSinceLastEnemySpawn = 0.0f;
            }
        }

        if (!bPendingBoss && !bDidBossStart)
        {
            TimeSinceLastEnemyFire += DeltaTime;
            bool DoNormal = DifficultyLevel <= 1;
            bool DoHeavy  = DifficultyLevel >= 1;
            bool CanFireNormal = DoNormal && Enemies.Num() > 0;
            bool CanFireHeavy  = DoHeavy  && HeavyEnemies.Num() > 0;
            if (TimeSinceLastEnemyFire >= EnemyFireInterval && (CanFireNormal || CanFireHeavy))
            {
                if (CanFireNormal)
                {
                    int32 MaxBullets = 3;
                    TArray<int32> AliveIndices;
                    for (int32 i = 0; i < Enemies.Num(); ++i)
                        if (Enemies[i].bIsAlive) AliveIndices.Add(i);
                    int32 BulletsToSpawn = FMath::Clamp(AliveIndices.Num(), 1, MaxBullets);
                    for (int32 shot = 0; shot < BulletsToSpawn && AliveIndices.Num() > 0; ++shot)
                    {
                        int32 RandomId = FMath::RandRange(0, AliveIndices.Num() - 1);
                        int32 EnemyIndex = AliveIndices[RandomId];
                        SpawnEnemyBullet(Enemies[EnemyIndex].Position, EnemyBulletTexture, FVector2D::ZeroVector);
                        AliveIndices.RemoveAt(RandomId);
                    }
                }
                if (CanFireHeavy)
                {
                    int32 MaxBullets = (DifficultyLevel == 2) ? 5 : 2;
                    TArray<int32> AliveIndices;
                    for (int32 i = 0; i < HeavyEnemies.Num(); ++i)
                        if (HeavyEnemies[i].bIsAlive) AliveIndices.Add(i);
                    int32 BulletsToSpawn = FMath::Clamp(AliveIndices.Num(), 1, MaxBullets);
                    for (int32 shot = 0; shot < BulletsToSpawn && AliveIndices.Num() > 0; ++shot)
                    {
                        int32 RandomId = FMath::RandRange(0, AliveIndices.Num() - 1);
                        int32 EnemyIndex = AliveIndices[RandomId];
                        SpawnHeavyEnemyBullet(HeavyEnemies[EnemyIndex].Position, HeavyEnemyBulletTexture, FVector2D::ZeroVector);
                        AliveIndices.RemoveAt(RandomId);
                    }
                }
                TimeSinceLastEnemyFire = 0.0f;
            }
        }

        // Boss phase: After 60s and all enemies dead
        if (!bDidBossStart && !bPendingBoss && GameElapsedTime >= 60.0f)
            bPendingBoss = true;
        if (bPendingBoss && !bDidBossStart && Enemies.Num() == 0 && HeavyEnemies.Num() == 0)
        {
            bDidBossStart = true;
            bPendingBoss = false;
            OnBossFightStart();
        }
    }

    UpdatePlayer(DeltaTime);
    UpdateEnemies(DeltaTime);
    UpdateHeavyEnemies(DeltaTime);
    UpdateProjectiles(DeltaTime);
    UpdateEnemyBullets(DeltaTime);
    UpdateHeavyEnemyBullets(DeltaTime);
    UpdateBoss(DeltaTime);
    CheckCollisions();
    UpdateWidget();
}

void UFirewallMiniGame::OnDifficultyIncrease(int32 NewDifficulty)
{
    // You may add SFX/UI here
}

void UFirewallMiniGame::UpdatePlayer(float DeltaTime)
{
    FVector2D Area = GetPlayAreaSize();
    if (!PlayerMoveInput.IsNearlyZero())
    {
        // Move both horizontally and vertically
        Player.Position.X += Player.MoveSpeed * PlayerMoveInput.X * DeltaTime;
        Player.Position.Y += Player.MoveSpeed * PlayerMoveInput.Y * DeltaTime;

        // Clamp to play field
        Player.Position.X = FMath::Clamp(Player.Position.X, Player.Size.X * 0.5f, Area.X - Player.Size.X * 0.5f);
        Player.Position.Y = FMath::Clamp(Player.Position.Y, Player.Size.Y * 0.5f, Area.Y - Player.Size.Y * 0.5f);
    }
}

void UFirewallMiniGame::UpdateWidget()
{
    if (!WidgetRef)
        return;
    WidgetRef->SetLives(Player.Lives);
    WidgetRef->SetBossHP(bIsBossActive ? BossHP : -1, BOSS_TOTAL_HP);  // New boss HP function
    WidgetRef->DrawGameObjects(Player, Enemies, HeavyEnemies, Projectiles, EnemyBullets, HeavyEnemyBullets, bIsBossActive, Boss);
}

void UFirewallMiniGame::GameOver()
{
    if (bIsGameOver)
        return;
    bIsGameOver = true;
    if (WidgetRef) WidgetRef->ShowGameOver();
    EndGame();
}

void UFirewallMiniGame::Victory()
{
    if (bIsGameOver)
        return;
    bIsGameOver = true;
    if (WidgetRef) WidgetRef->ShowVictory();
    EndGame();
}

void UFirewallMiniGame::OnMoveAxis(const FInputActionValue& Value)
{
    // Value.Get<FVector2D>() is the Enhanced Input way to get 2D axis
    PlayerMoveInput = Value.Get<FVector2D>();
}

void UFirewallMiniGame::OnMoveAxisCompleted(const FInputActionValue& Value)
{
    PlayerMoveInput = FVector2D::ZeroVector;
}