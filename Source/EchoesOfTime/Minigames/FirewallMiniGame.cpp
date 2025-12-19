#include "FirewallMiniGame.h"
#include "Widgets/Minigames/FirewallWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/World.h"

UFirewallMiniGame::UFirewallMiniGame()
    : Score(0)
    , bIsGameOver(false)
    , EnemySpawnInterval(0.8f)
    , TimeSinceLastEnemySpawn(0.0f)
    , EnemyFireInterval(1.0f) // Fires frequently for testing
    , TimeSinceLastEnemyFire(0.0f)
    , WidgetRef(nullptr)
    , OwningController(nullptr)
{}

FVector2D UFirewallMiniGame::GetPlayAreaSize() const
{
    if (WidgetRef && WidgetRef->GameCanvas)
    {
        return WidgetRef->GameCanvas->GetCachedGeometry().GetLocalSize();
    }
    return FVector2D(1920, 1080);
}

void UFirewallMiniGame::StartGame(APlayerController* PlayerController)
{
    if (!PlayerController)
        return;

    OwningController = PlayerController;

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwningController->GetLocalPlayer()))
    {
        if (GameplayIMC)
            Subsystem->RemoveMappingContext(GameplayIMC);
        if (FirewallIMC)
            Subsystem->AddMappingContext(FirewallIMC, 100);
    }

    Score = 0;
    bIsGameOver = false;
    TimeSinceLastEnemySpawn = 0.0f;
    TimeSinceLastEnemyFire = 0.0f;

    CreateWidget();

    if (UWorld* World = OwningController->GetWorld())
    {
        World->GetTimerManager().SetTimerForNextTick(this, &UFirewallMiniGame::TryFinishInitWhenCanvasReady);
    }
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
            {
                World->GetTimerManager().SetTimerForNextTick(this, &UFirewallMiniGame::TryFinishInitWhenCanvasReady);
            }
        }
    }
}

void UFirewallMiniGame::FinishInitAfterWidgetReady()
{
    FVector2D Area = GetPlayAreaSize();
    Player = FMiniGamePlayer();
    Player.Position = FVector2D(Area.X * 0.5f, Area.Y - Area.Y * 0.12f);
    Player.Texture = PlayerTexture;
    Player.Size = FVector2D(75, 95);
    Player.MoveSpeed = Area.X * 0.4f;

    Enemies.Empty();
    Projectiles.Empty();
    EnemyBullets.Empty();

    SetupInput();

    if (UWorld* World = OwningController->GetWorld())
    {
        World->GetTimerManager().SetTimer(
            TickTimerHandle,
            this,
            &UFirewallMiniGame::TickGame,
            0.016f,
            true
        );
    }

    UpdateWidget();
}

void UFirewallMiniGame::SpawnEnemy()
{
    FVector2D Area = GetPlayAreaSize();
    float MinX = Area.X * 0.06f;
    float MaxX = Area.X * 0.94f;

    FMiniGameEnemy Enemy;
    float RandX = FMath::RandRange(MinX, MaxX);
    Enemy.Position = FVector2D(RandX, 0.f + 32.0f);
    Enemy.Texture = EnemyTexture;
    Enemy.bIsAlive = true;
    Enemy.Size = FVector2D(75, 75);
    Enemies.Add(Enemy);
}

void UFirewallMiniGame::SpawnPlayerBullet()
{
    FMiniGameProjectile Projectile;
    Projectile.Position = Player.Position;
    Projectile.Texture = ProjectileTexture;
    Projectile.Size = FVector2D(8, 54);
    Projectile.bIsActive = true;
    Projectiles.Add(Projectile);
}

void UFirewallMiniGame::SpawnEnemyBullet(const FVector2D& EnemyPosition)
{
    FMiniGameEnemyBullet Bullet;
    Bullet.Position = EnemyPosition;
    Bullet.Texture = EnemyBulletTexture;
    Bullet.Size = FVector2D(40, 38);
    Bullet.bIsActive = true;
    EnemyBullets.Add(Bullet);
}

void UFirewallMiniGame::UpdateEnemies(float DeltaTime)
{
    FVector2D Area = GetPlayAreaSize();
    float FallSpeed = Area.Y * 0.20f;

    for (int32 i = 0; i < Enemies.Num(); ++i)
    {
        FMiniGameEnemy& Enemy = Enemies[i];
        if (!Enemy.bIsAlive) continue;

        Enemy.Position.Y += FallSpeed * DeltaTime;

        if (Enemy.Position.Y - Enemy.Size.Y * 0.5f > Area.Y)
        {
            Enemy.bIsAlive = false;
        }
    }
    Enemies.RemoveAll([](const FMiniGameEnemy& Enemy) { return !Enemy.bIsAlive; });
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
        {
            Projectile.bIsActive = false;
        }
    }
    Projectiles.RemoveAll([](const FMiniGameProjectile& P) { return !P.bIsActive; });
}

void UFirewallMiniGame::UpdateEnemyBullets(float DeltaTime)
{
    FVector2D Area = GetPlayAreaSize();
    float BulletSpeed = Area.Y * 0.38f;

    for (int32 i = 0; i < EnemyBullets.Num(); ++i)
    {
        FMiniGameEnemyBullet& Bull = EnemyBullets[i];
        if (!Bull.bIsActive) continue;

        Bull.Position.Y += BulletSpeed * DeltaTime;

        if (Bull.Position.Y > Area.Y + Bull.Size.Y * 0.8f)
        {
            Bull.bIsActive = false;
        }
    }
    EnemyBullets.RemoveAll([](const FMiniGameEnemyBullet& B) { return !B.bIsActive; });
}

void UFirewallMiniGame::CheckCollisions()
{
    FVector2D Area = GetPlayAreaSize();
    float PlayerCollisionRadius = FMath::Max(Player.Size.X, Player.Size.Y) * 0.5f;

    // Player projectiles hit enemies
    for (int32 p = 0; p < Projectiles.Num(); ++p)
    {
        FMiniGameProjectile& Projectile = Projectiles[p];
        if (!Projectile.bIsActive) continue;

        // Collide with enemy
        for (int32 e = 0; e < Enemies.Num(); ++e)
        {
            FMiniGameEnemy& Enemy = Enemies[e];
            if (!Enemy.bIsAlive) continue;

            float Distance = FVector2D::Distance(Projectile.Position, Enemy.Position);
            float CollisionRadius = FMath::Max(Enemy.Size.X, Enemy.Size.Y) * 0.42f;
            if (Distance < CollisionRadius)
            {
                Enemy.bIsAlive = false;
                Projectile.bIsActive = false;
                Score += 10;
                break;
            }
        }

        // Collide with enemy bullets (NEW: destroy both)
        for (int32 b = 0; b < EnemyBullets.Num(); ++b)
        {
            FMiniGameEnemyBullet& EnemyBullet = EnemyBullets[b];
            if (!EnemyBullet.bIsActive) continue;

            float Distance = FVector2D::Distance(Projectile.Position, EnemyBullet.Position);
            float CollisionRadius = FMath::Max(Projectile.Size.X, Projectile.Size.Y) * 0.5f +
                                   FMath::Max(EnemyBullet.Size.X, EnemyBullet.Size.Y) * 0.5f;
            if (Distance < CollisionRadius * 0.7f) // tuned - reduce if too forgiving
            {
                Projectile.bIsActive = false;
                EnemyBullet.bIsActive = false;
                break; // bullet destroyed, go to next
            }
        }
    }

    // Enemy bullets hit player
    for (int32 b = 0; b < EnemyBullets.Num(); ++b)
    {
        FMiniGameEnemyBullet& Bull = EnemyBullets[b];
        if (!Bull.bIsActive) continue;
        float Distance = FVector2D::Distance(Bull.Position, Player.Position);
        if (Distance < PlayerCollisionRadius)
        {
            Bull.bIsActive = false;
            Player.Lives -= 1;
            if (Player.Lives <= 0)
            {
                GameOver();
            }
        }
    }
}

void UFirewallMiniGame::EndGame()
{
    if (OwningController)
    {
        if (UWorld* World = OwningController->GetWorld())
        {
            World->GetTimerManager().ClearTimer(TickTimerHandle);
        }

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
    Projectiles.Empty();
    EnemyBullets.Empty();
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

void UFirewallMiniGame::OnMoveAxisCompleted(const FInputActionValue& Value)
{
    PlayerMoveInput = 0.0f;
}

void UFirewallMiniGame::CleanupInput()
{
    // Function ready if input cleanup is required
}

void UFirewallMiniGame::TickGame()
{
    if (bIsGameOver)
        return;

    float DeltaTime = 0.016f; // 60hz tick

    // Enemy spawn logic
    TimeSinceLastEnemySpawn += DeltaTime;
    if (TimeSinceLastEnemySpawn >= EnemySpawnInterval)
    {
        SpawnEnemy();
        TimeSinceLastEnemySpawn = 0.0f;
    }

    // Enemy bullet fire logic: pick N random alive enemies, NOT just one random enemy
    TimeSinceLastEnemyFire += DeltaTime;
    if (TimeSinceLastEnemyFire >= EnemyFireInterval && Enemies.Num() > 0)
    {
        // How many bullets per volley? Set to Enemies.Num(), or clamp
        int32 BulletsToSpawn = FMath::Clamp(Enemies.Num(), 1, 3); // Up to 3 per volley for chaos; raise if you want!
        TArray<int32> AliveIndices;
        for (int32 i = 0; i < Enemies.Num(); ++i)
        {
            if (Enemies[i].bIsAlive)
                AliveIndices.Add(i);
        }

        for (int32 shot = 0; shot < BulletsToSpawn && AliveIndices.Num() > 0; ++shot)
        {
            int32 RandomId = FMath::RandRange(0, AliveIndices.Num() - 1);
            int32 EnemyIndex = AliveIndices[RandomId];
            SpawnEnemyBullet(Enemies[EnemyIndex].Position);
            AliveIndices.RemoveAt(RandomId); // Don't choose the same enemy more than once for this volley
        }

        TimeSinceLastEnemyFire = 0.0f;
    }

    UpdatePlayer(DeltaTime);
    UpdateEnemies(DeltaTime);
    UpdateProjectiles(DeltaTime);
    UpdateEnemyBullets(DeltaTime);
    CheckCollisions();
    UpdateWidget();
}

void UFirewallMiniGame::UpdatePlayer(float DeltaTime)
{
    FVector2D Area = GetPlayAreaSize();
    if (FMath::Abs(PlayerMoveInput) > KINDA_SMALL_NUMBER) {
        Player.Position.X += Player.MoveSpeed * PlayerMoveInput * DeltaTime;
        Player.Position.X = FMath::Clamp(Player.Position.X, Player.Size.X * 0.5f, Area.X - Player.Size.X * 0.5f);
    }
}

void UFirewallMiniGame::UpdateWidget()
{
    if (!WidgetRef)
        return;
    WidgetRef->SetScore(Score);
    WidgetRef->SetLives(Player.Lives);
    WidgetRef->DrawGameObjects(Player, Enemies, Projectiles, EnemyBullets);
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
    PlayerMoveInput = Value.Get<float>();
}