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
    , bMovingLeft(false)
    , bMovingRight(false)
    , WidgetRef(nullptr)
    , OwningController(nullptr)
{
    UE_LOG(LogTemp, Warning, TEXT("UFirewallMiniGame: Constructor called"));
}

FVector2D UFirewallMiniGame::GetPlayAreaSize() const
{
    if (WidgetRef && WidgetRef->GameCanvas)
    {
        FVector2D Size = WidgetRef->GameCanvas->GetCachedGeometry().GetLocalSize();
        UE_LOG(LogTemp, Warning, TEXT("GetPlayAreaSize: WidgetRef/GameCanvas size = (%.1f, %.1f)"), Size.X, Size.Y);
        return Size;
    }
    UE_LOG(LogTemp, Error, TEXT("GetPlayAreaSize: Fallback value used"));
    return FVector2D(1920, 1080);
}

void UFirewallMiniGame::StartGame(APlayerController* PlayerController)
{
    UE_LOG(LogTemp, Warning, TEXT("StartGame called"));
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("StartGame: PlayerController is null!"));
        return;
    }

    OwningController = PlayerController;

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwningController->GetLocalPlayer()))
    {
        if (GameplayIMC)
        {
            UE_LOG(LogTemp, Warning, TEXT("StartGame: Removing GameplayIMC"));
            Subsystem->RemoveMappingContext(GameplayIMC);
        }
        if (FirewallIMC)
        {
            UE_LOG(LogTemp, Warning, TEXT("StartGame: Adding FirewallIMC"));
            Subsystem->AddMappingContext(FirewallIMC, 100);
        }
    }

    Score = 0;
    bIsGameOver = false;
    EnemyMoveDirection = 1.0f;
    bMovingLeft = false;
    bMovingRight = false;

    CreateWidget();

    // Use deferred polling for geometry size
    if (UWorld* World = OwningController->GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("StartGame: Setting timer for TryFinishInitWhenCanvasReady"));
        World->GetTimerManager().SetTimerForNextTick(this, &UFirewallMiniGame::TryFinishInitWhenCanvasReady);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("StartGame: World is null!"));
    }
}

void UFirewallMiniGame::TryFinishInitWhenCanvasReady()
{
    bool bReady = false;
    if (WidgetRef && WidgetRef->GameCanvas)
    {
        FVector2D Area = WidgetRef->GameCanvas->GetCachedGeometry().GetLocalSize();
        UE_LOG(LogTemp, Warning, TEXT("TryFinishInitWhenCanvasReady: Canvas size = (%.1f, %.1f)"), Area.X, Area.Y);
        bReady = (Area.X > 0.0f && Area.Y > 0.0f);
    }
    if (bReady)
    {
        FinishInitAfterWidgetReady();
    }
    else
    {
        // Not ready, poll again next tick
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
    UE_LOG(LogTemp, Warning, TEXT("FinishInitAfterWidgetReady called"));

    FVector2D Area = GetPlayAreaSize();

    // Setup player - Relative to play area
    Player = FMiniGamePlayer();
    Player.Position = FVector2D(Area.X * 0.5f, Area.Y - Area.Y * 0.12f);
    Player.Texture = PlayerTexture;
    Player.Size = FVector2D(Area.X * 0.06f, Area.Y * 0.10f);
    Player.MoveSpeed = Area.X * 0.4f;
    UE_LOG(LogTemp, Warning, TEXT("FinishInitAfterWidgetReady: Player at (%.1f, %.1f), size=(%.1f, %.1f), speed=%.1f, texture=%s"), Player.Position.X, Player.Position.Y, Player.Size.X, Player.Size.Y, Player.MoveSpeed, Player.Texture ? *Player.Texture->GetName() : TEXT("NULL"));

    Enemies.Empty();
    Projectiles.Empty();
    SpawnEnemies();

    SetupInput();

    if (UWorld* World = OwningController->GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("FinishInitAfterWidgetReady: Setting TickGame timer"));
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
    UE_LOG(LogTemp, Warning, TEXT("SpawnEnemies called"));

    FVector2D Area = GetPlayAreaSize();
    int32 Rows = 5;
    int32 Columns = 11;

    float SpacingX = Area.X / 15.0f;
    float SpacingY = Area.Y / 18.0f;
    float EnemySizeX = Area.X * 0.05f;
    float EnemySizeY = Area.Y * 0.075f;

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
            Enemy.Size = FVector2D(EnemySizeX, EnemySizeY);

            Enemies.Add(Enemy);

            UE_LOG(LogTemp, Warning, TEXT("SpawnEnemies: Enemy[%d,%d] at (%.1f, %.1f), size=(%.1f, %.1f), texture=%s"), Row, Col, Enemy.Position.X, Enemy.Position.Y, Enemy.Size.X, Enemy.Size.Y, Enemy.Texture ? *Enemy.Texture->GetName() : TEXT("NULL"));
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("SpawnEnemies: Total enemies spawned: %d"), Enemies.Num());
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

        UE_LOG(LogTemp, Verbose, TEXT("UpdateEnemies: Enemy[%d] Position=(%.1f,%.1f) MoveDir=%.1f MoveSpeed=%.1f Delta=%.4f"),
            i, Enemy.Position.X, Enemy.Position.Y, EnemyMoveDirection, MoveSpeed, DeltaTime);
    }

    if (bShouldMoveDown)
    {
        EnemyMoveDirection *= -1.0f;
        float DownAmount = Area.Y * 0.035f;
        UE_LOG(LogTemp, Warning, TEXT("UpdateEnemies: Moving down by %.2f, reversing direction"), DownAmount);
        for (int32 i = 0; i < Enemies.Num(); ++i)
        {
            FMiniGameEnemy& Enemy = Enemies[i];
            Enemy.Position.Y += DownAmount;
            if (Enemy.Position.Y >= Area.Y - Area.Y * 0.17f)
            {
                UE_LOG(LogTemp, Warning, TEXT("UpdateEnemies: Enemy[%d] reached bottom, GameOver"), i);
                GameOver();
            }
        }
        for (int32 i = 0; i < Enemies.Num(); ++i)
        {
            FMiniGameEnemy& Enemy = Enemies[i];
            Enemy.Position.X = FMath::Clamp(Enemy.Position.X, Area.X * 0.06f, Area.X * 0.94f);
        }
    }
}

void UFirewallMiniGame::OnFirePressed()
{
    UE_LOG(LogTemp, Warning, TEXT("OnFirePressed"));

    if (bIsGameOver)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnFirePressed: Game is over, ignoring fire"));
        return;
    }

    FVector2D Area = GetPlayAreaSize();
    FMiniGameProjectile Projectile;
    Projectile.Position = Player.Position;
    Projectile.Texture = ProjectileTexture;
    Projectile.bIsPlayerProjectile = true;
    Projectile.bIsActive = true;
    Projectile.Size = FVector2D(Area.X * 0.012f, Area.Y * 0.035f);

    Projectiles.Add(Projectile);

    UE_LOG(LogTemp, Warning, TEXT("OnFirePressed: Projectile spawned at (%.1f, %.1f) size=(%.1f, %.1f) texture=%s"), Projectile.Position.X, Projectile.Position.Y, Projectile.Size.X, Projectile.Size.Y, Projectile.Texture ? *Projectile.Texture->GetName() : TEXT("NULL"));
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
            UE_LOG(LogTemp, Warning, TEXT("UpdateProjectiles: Projectile[%d] went offscreen"), i);
        }
        else
        {
            UE_LOG(LogTemp, Verbose, TEXT("UpdateProjectiles: Projectile[%d] moved to (%.1f, %.1f)"), i, Projectile.Position.X, Projectile.Position.Y);
        }
    }
    int32 Before = Projectiles.Num();
    Projectiles.RemoveAll([](const FMiniGameProjectile& P) { return !P.bIsActive; });
    int32 After = Projectiles.Num();
    if (Before != After)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateProjectiles: Removed %d inactive projectiles"), Before - After);
    }
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
                UE_LOG(LogTemp, Warning, TEXT("CheckCollisions: Projectile[%d] hit Enemy[%d]"), p, e);
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
    {
        UE_LOG(LogTemp, Warning, TEXT("CheckCollisions: All enemies dead, Victory!"));
        Victory();
    }
}

void UFirewallMiniGame::EndGame()
{
    UE_LOG(LogTemp, Warning, TEXT("EndGame called"));
    if (OwningController)
    {
        if (UWorld* World = OwningController->GetWorld())
        {
            World->GetTimerManager().ClearTimer(TickTimerHandle);
            UE_LOG(LogTemp, Warning, TEXT("EndGame: Cleared TickGame timer"));
        }

        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwningController->GetLocalPlayer()))
        {
            if (FirewallIMC)
            {
                UE_LOG(LogTemp, Warning, TEXT("EndGame: Removing FirewallIMC"));
                Subsystem->RemoveMappingContext(FirewallIMC);
            }
            if (GameplayIMC)
            {
                UE_LOG(LogTemp, Warning, TEXT("EndGame: Restoring GameplayIMC"));
                Subsystem->AddMappingContext(GameplayIMC, 0);
            }
        }
    }
    if (WidgetRef)
    {
        WidgetRef->RemoveFromParent();
        WidgetRef = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("EndGame: Removed WidgetRef from viewport"));
    }
    bIsGameOver = true;
    CleanupInput();
    Enemies.Empty();
    Projectiles.Empty();
    OwningController = nullptr;
    UE_LOG(LogTemp, Warning, TEXT("EndGame: Cleaned up members"));
}

void UFirewallMiniGame::CreateWidget()
{
    UE_LOG(LogTemp, Warning, TEXT("CreateWidget called"));
    if (!OwningController)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateWidget: OwningController is null!"));
        return;
    }
    if (!WidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateWidget: WidgetClass is null!"));
        return;
    }

    WidgetRef = ::CreateWidget<UFirewallWidget>(OwningController, WidgetClass);
    if (WidgetRef)
    {
        WidgetRef->AddToViewport(0);
        UE_LOG(LogTemp, Warning, TEXT("CreateWidget: WidgetRef added to viewport"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CreateWidget: Failed to create WidgetRef!"));
    }
}

void UFirewallMiniGame::SetupInput()
{
    UE_LOG(LogTemp, Warning, TEXT("SetupInput called"));
    if (!OwningController)
    {
        UE_LOG(LogTemp, Error, TEXT("SetupInput: OwningController is null!"));
        return;
    }
    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(OwningController->InputComponent);
    if (EnhancedInput)
    {
        EnhancedInput->BindAction(MoveLeftAction, ETriggerEvent::Started, this, &UFirewallMiniGame::OnMoveLeftPressed);
        EnhancedInput->BindAction(MoveLeftAction, ETriggerEvent::Completed, this, &UFirewallMiniGame::OnMoveLeftReleased);
        EnhancedInput->BindAction(MoveRightAction, ETriggerEvent::Started, this, &UFirewallMiniGame::OnMoveRightPressed);
        EnhancedInput->BindAction(MoveRightAction, ETriggerEvent::Completed, this, &UFirewallMiniGame::OnMoveRightReleased);
        EnhancedInput->BindAction(FireAction, ETriggerEvent::Started, this, &UFirewallMiniGame::OnFirePressed);
        UE_LOG(LogTemp, Warning, TEXT("SetupInput: Actions bound"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SetupInput: EnhancedInput is null!"));
    }
}

void UFirewallMiniGame::CleanupInput()
{
    UE_LOG(LogTemp, Warning, TEXT("CleanupInput called"));
    // You can add extra input cleanup here if needed
}

void UFirewallMiniGame::TickGame()
{
    if (bIsGameOver)
    {
        UE_LOG(LogTemp, Warning, TEXT("TickGame: Game is over, skipping tick"));
        return;
    }
    float DeltaTime = 0.016f;

    UE_LOG(LogTemp, Verbose, TEXT("TickGame: DeltaTime=%.4f"), DeltaTime);

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
    if (bMovingLeft)
    {
        Player.Position.X -= Player.MoveSpeed * DeltaTime;
        Player.Position.X = FMath::Max(Player.Size.X * 0.5f, Player.Position.X);
    }
    if (bMovingRight)
    {
        Player.Position.X += Player.MoveSpeed * DeltaTime;
        Player.Position.X = FMath::Min(Area.X - Player.Size.X * 0.5f, Player.Position.X);
    }
    if (OldX != Player.Position.X)
    {
        UE_LOG(LogTemp, Verbose, TEXT("UpdatePlayer: Player.X moved from %.1f to %.1f"), OldX, Player.Position.X);
    }
}

void UFirewallMiniGame::UpdateWidget()
{
    UE_LOG(LogTemp, Warning, TEXT("UpdateWidget called. Score=%d Lives=%d"), Score, Player.Lives);
    if (!WidgetRef)
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateWidget: WidgetRef is null!"));
        return;
    }
    WidgetRef->SetScore(Score);
    WidgetRef->SetLives(Player.Lives);
    WidgetRef->DrawGameObjects(Player, Enemies, Projectiles);
}

void UFirewallMiniGame::GameOver()
{
    if (bIsGameOver)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameOver called but game already over"));
        return;
    }
    bIsGameOver = true;
    UE_LOG(LogTemp, Warning, TEXT("GameOver: calling WidgetRef->ShowGameOver"));
    if (WidgetRef) WidgetRef->ShowGameOver();
    EndGame();
}

void UFirewallMiniGame::Victory()
{
    if (bIsGameOver)
    {
        UE_LOG(LogTemp, Warning, TEXT("Victory called but game already over"));
        return;
    }
    bIsGameOver = true;
    UE_LOG(LogTemp, Warning, TEXT("Victory: calling WidgetRef->ShowVictory"));
    if (WidgetRef) WidgetRef->ShowVictory();
    EndGame();
}

// Input callbacks
void UFirewallMiniGame::OnMoveLeftPressed() { bMovingLeft = true; UE_LOG(LogTemp, Warning, TEXT("OnMoveLeftPressed")); }
void UFirewallMiniGame::OnMoveLeftReleased() { bMovingLeft = false; UE_LOG(LogTemp, Warning, TEXT("OnMoveLeftReleased")); }
void UFirewallMiniGame::OnMoveRightPressed() { bMovingRight = true; UE_LOG(LogTemp, Warning, TEXT("OnMoveRightPressed")); }
void UFirewallMiniGame::OnMoveRightReleased() { bMovingRight = false; UE_LOG(LogTemp, Warning, TEXT("OnMoveRightReleased")); }