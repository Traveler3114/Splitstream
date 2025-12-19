// FirewallMiniGame.cpp

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
    , EnemyMoveDirection(1.0f)
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
    EnemyMoveDirection = 1.0f;

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
    SpawnEnemies();

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

void UFirewallMiniGame::SpawnEnemies()
{
    FVector2D Area = GetPlayAreaSize();
    int32 Rows = 5;
    int32 Columns = 11;

    float SpacingX = Area.X / 15.0f;
    float SpacingY = Area.Y / 18.0f;
    float TotalWidth = (Columns - 1) * SpacingX;
    float StartX = (Area.X - TotalWidth) * 0.5f;
    float StartY = Area.Y * 0.11f;

    for (int32 Row = 0; Row < Rows; ++Row)
    {
        for (int32 Col = 0; Col < Columns; ++Col)
        {
            FMiniGameEnemy Enemy;
            Enemy.Position = FVector2D(StartX + Col * SpacingX, StartY + Row * SpacingY);
            Enemy.Texture = EnemyTexture;
            Enemy.bIsAlive = true;
            Enemy.Size = FVector2D(75, 75);
            Enemies.Add(Enemy);
        }
    }
}

void UFirewallMiniGame::UpdateEnemies(float DeltaTime)
{
    FVector2D Area = GetPlayAreaSize();
    float MoveSpeed = Area.X * 0.35f;
    bool bShouldMoveDown = false;

    for (int32 i = 0; i < Enemies.Num(); ++i)
    {
        FMiniGameEnemy& Enemy = Enemies[i];
        if (!Enemy.bIsAlive) continue;

        Enemy.Position.X += EnemyMoveDirection * MoveSpeed * DeltaTime;

        if (Enemy.Position.X <= Area.X * 0.06f || Enemy.Position.X >= Area.X * 0.94f)
            bShouldMoveDown = true;
    }

    if (bShouldMoveDown)
    {
        EnemyMoveDirection *= -1.0f;
        float DownAmount = Area.Y * 0.035f;
        for (int32 i = 0; i < Enemies.Num(); ++i)
        {
            FMiniGameEnemy& Enemy = Enemies[i];
            Enemy.Position.Y += DownAmount;
            if (Enemy.Position.Y >= Area.Y - Area.Y * 0.17f)
                GameOver();
            Enemy.Position.X = FMath::Clamp(Enemy.Position.X, Area.X * 0.06f, Area.X * 0.94f);
        }
    }
}

void UFirewallMiniGame::OnFirePressed()
{
    if (bIsGameOver)
        return;

    FMiniGameProjectile Projectile;
    Projectile.Position = Player.Position;
    Projectile.Texture = ProjectileTexture;
    Projectile.bIsActive = true;
    Projectile.Size = FVector2D(8, 54);

    Projectiles.Add(Projectile);
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

void UFirewallMiniGame::CheckCollisions()
{
    FVector2D Area = GetPlayAreaSize();
    float CollisionRadius = Area.X * 0.03f;

    for (int32 p = 0; p < Projectiles.Num(); ++p)
    {
        FMiniGameProjectile& Projectile = Projectiles[p];
        if (!Projectile.bIsActive) continue;

        for (int32 e = 0; e < Enemies.Num(); ++e)
        {
            FMiniGameEnemy& Enemy = Enemies[e];
            if (!Enemy.bIsAlive) continue;

            float Distance = FVector2D::Distance(Projectile.Position, Enemy.Position);
            if (Distance < CollisionRadius)
            {
                Enemy.bIsAlive = false;
                Projectile.bIsActive = false;
                Score += 10;
                break;
            }
        }
    }

    bool bAnyAlive = false;
    for (int32 i = 0; i < Enemies.Num(); ++i)
    {
        if (Enemies[i].bIsAlive)
        {
            bAnyAlive = true;
            break;
        }
    }
    if (!bAnyAlive)
        Victory();
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

    float DeltaTime = 0.016f;

    UpdatePlayer(DeltaTime);
    UpdateEnemies(DeltaTime);
    UpdateProjectiles(DeltaTime);
    CheckCollisions();
    UpdateWidget();
}

void UFirewallMiniGame::UpdatePlayer(float DeltaTime)
{
    FVector2D Area = GetPlayAreaSize();
    float OldX = Player.Position.X;
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
    WidgetRef->DrawGameObjects(Player, Enemies, Projectiles);
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