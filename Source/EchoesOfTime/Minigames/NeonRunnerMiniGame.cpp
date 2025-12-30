#include "NeonRunnerMiniGame.h"
#include "Widgets/Minigames/NeonRunnerWidget.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"

UNeonRunnerMiniGame::UNeonRunnerMiniGame()
    : TimeSinceLastObstacle(0.f), NextObsDistance(0.f), SurvivalTime(0.f),
    bIsGameOver(false), WidgetRef(nullptr), OwningController(nullptr), CurrentMoveSpeed(720.f)
{
    PlayerStartX = 320.f;
}

FVector2D UNeonRunnerMiniGame::GetPlayAreaSize() const
{
    if (WidgetRef && WidgetRef->GameCanvas)
        return WidgetRef->GameCanvas->GetCachedGeometry().GetLocalSize();
    return FVector2D(1920, 1080);
}

FVector2D UNeonRunnerMiniGame::GetTextureSize(UTexture2D* Texture) const
{
    if (Texture)
        return FVector2D(Texture->GetImportedSize().X, Texture->GetImportedSize().Y);
    return FVector2D(96, 96);
}

void UNeonRunnerMiniGame::StartGame(APlayerController* PlayerController)
{
    OwningController = PlayerController;
    bIsGameOver = false;
    SurvivalTime = 0.f;
    TimeSinceLastObstacle = 0.f;
    NextObsDistance = FMath::FRandRange(700.f, 1050.f);

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwningController->GetLocalPlayer()))
    {
        if (GameplayIMC) Subsystem->RemoveMappingContext(GameplayIMC);
        if (RunnerIMC) Subsystem->AddMappingContext(RunnerIMC, 100);
    }

    Player = FNeonRunnerPlayer();
    Player.Texture = RunnerTexture;
    Player.Size = GetTextureSize(RunnerTexture);
    Player.Position = FVector2D(PlayerStartX, GroundY - Player.Size.Y * 0.5f);
    Player.Velocity = FVector2D::ZeroVector;
    Player.bIsOnGround = true;

    Obstacles.Empty();
    Tiles.Empty();

    FVector2D Area = GetPlayAreaSize();
    float width = Area.X > 0 ? Area.X : 1920.f;
    int numTiles = FMath::CeilToInt(width / TileSize) + 4;
    for (int i = 0; i < numTiles; ++i) {
        FNeonRunnerObstacle tile;
        tile.Type = EObstacleType::Tile;
        tile.Texture = TileTexture;
        tile.Size = FVector2D(TileSize, TileSize * 0.7f);
        tile.Position = FVector2D(i * TileSize + TileSize * 0.5f, GroundY + TileSize * 0.15f);
        tile.bIsActive = true;
        Tiles.Add(tile);
    }

    CreateWidget();

    if (UWorld* World = OwningController->GetWorld())
        World->GetTimerManager().SetTimerForNextTick(this, &UNeonRunnerMiniGame::TickGame);

    SetupInput();
    UpdateWidget();
}

void UNeonRunnerMiniGame::TickGame()
{
    if (bIsGameOver) return;

    float DeltaTime = 0.016f;
    SurvivalTime += DeltaTime;

    // --- Gradual speed-up by progress, using Blueprint variable ---
    float ProgressRatio = SurvivalTime / VictoryTime;
    ProgressRatio = FMath::Clamp(ProgressRatio, 0.f, 1.f);

    // Calculate percent [0, 100]
    float percent = ProgressRatio * 100.0f;
    // Start speed is 720, then add user-chosen increment per percent (default 3.8)
    CurrentMoveSpeed = 720.f + SpeedIncreasePerPercent * percent;
    // Clamp to 1100.f maximum
    if (CurrentMoveSpeed > 1100.f) CurrentMoveSpeed = 1100.f;

    float totalScroll = CurrentMoveSpeed * DeltaTime;
    for (FNeonRunnerObstacle& Tile : Tiles)
    {
        Tile.Position.X -= totalScroll;
        if (Tile.Position.X + Tile.Size.X * 0.5f < -100.f)
            Tile.bIsActive = false;
    }
    Tiles.RemoveAll([](const FNeonRunnerObstacle& T) { return !T.bIsActive; });

    FVector2D Area = GetPlayAreaSize();
    float MostRight = 0.f;
    for (const FNeonRunnerObstacle& Tile : Tiles)
        if (Tile.Position.X > MostRight)
            MostRight = Tile.Position.X;
    while (MostRight < Area.X + TileSize * 2) {
        FNeonRunnerObstacle tile;
        tile.Type = EObstacleType::Tile;
        tile.Texture = TileTexture;
        tile.Size = FVector2D(TileSize, TileSize * 0.7f);
        tile.Position = FVector2D(MostRight + TileSize, GroundY + TileSize * 0.15f);
        tile.bIsActive = true;
        Tiles.Add(tile);
        MostRight = tile.Position.X;
    }

    for (FNeonRunnerObstacle& Obs : Obstacles)
    {
        Obs.Position.X -= CurrentMoveSpeed * DeltaTime;
        if (Obs.Position.X + Obs.Size.X < 0.f)
            Obs.bIsActive = false;
    }
    Obstacles.RemoveAll([](const FNeonRunnerObstacle& Obs) { return !Obs.bIsActive; });

    // Shorten gap slightly at higher difficulties (harder)
    TimeSinceLastObstacle += CurrentMoveSpeed * DeltaTime;
    if (TimeSinceLastObstacle >= NextObsDistance)
    {
        float baseMin = 700.f;
        float baseMax = 1050.f;
        float modifier = 1.0f - ProgressRatio * 0.45f; // 20-45% shorter/harder at endgame
        NextObsDistance = FMath::FRandRange(baseMin, baseMax) * modifier;

        SpawnObstacle();
        TimeSinceLastObstacle = 0.f;
    }

    UpdatePlayer(DeltaTime);
    CheckCollisions();
    UpdateWidget();

    if (!bIsGameOver && OwningController && OwningController->GetWorld())
        OwningController->GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UNeonRunnerMiniGame::TickGame);
}

void UNeonRunnerMiniGame::SpawnObstacle()
{
    FVector2D Area = GetPlayAreaSize();
    float baseX = Area.X + 80.0f;

    float ProgressRatio = SurvivalTime / VictoryTime;
    ProgressRatio = FMath::Clamp(ProgressRatio, 0.f, 1.f);

    int minSpikes = 1, maxSpikes = 2;
    if (ProgressRatio < 0.25f) {
        minSpikes = 1; maxSpikes = 2;
    }
    else if (ProgressRatio < 0.5f) {
        minSpikes = 2; maxSpikes = 3;
    }
    else if (ProgressRatio < 0.75f) {
        minSpikes = 3; maxSpikes = 4;
    }
    else {
        minSpikes = 4; maxSpikes = 4;
    }
    int groupCount = FMath::RandRange(minSpikes, maxSpikes);

    FVector2D spikeSize = GetTextureSize(ObstacleTexture_Spike);
    float SpikeGap = spikeSize.X * 0.1f;

    float lastRight = baseX;
    for (int i = 0; i < groupCount; ++i)
    {
        FNeonRunnerObstacle spike;
        spike.Type = EObstacleType::Spike;
        spike.Texture = ObstacleTexture_Spike;
        spike.Size = spikeSize;

        spike.Position = FVector2D(
            lastRight + spikeSize.X * 0.5f + (i > 0 ? SpikeGap : 0.0f),
            GroundY - spikeSize.Y * 0.5f + SpikeFloorAlignOffset
        );
        spike.bIsActive = true;
        Obstacles.Add(spike);

        lastRight = spike.Position.X + spikeSize.X * 0.5f + SpikeGap;
    }
}

void UNeonRunnerMiniGame::UpdatePlayer(float DeltaTime)
{
    FVector2D HalfSizeP = Player.Size * 0.5f;
    bool WasOnGround = Player.bIsOnGround;

    if (!Player.bIsOnGround)
        Player.Velocity.Y += Gravity * DeltaTime;
    float AttemptY = Player.Position.Y + Player.Velocity.Y * DeltaTime;

    // --- Ground check ---
    float PlayerFootLeft = Player.Position.X - HalfSizeP.X * 0.8f;
    float PlayerFootRight = Player.Position.X + HalfSizeP.X * 0.8f;

    float DesiredY = AttemptY;
    float BestTileTop = -FLT_MAX;
    bool bFoundGround = false;

    for (const FNeonRunnerObstacle& Tile : Tiles)
    {
        if (!Tile.bIsActive) continue;
        float TileLeft = Tile.Position.X - Tile.Size.X * 0.5f;
        float TileRight = Tile.Position.X + Tile.Size.X * 0.5f;
        float TileTop = Tile.Position.Y - Tile.Size.Y * 0.5f;

        float OverlapLeft = FMath::Max(PlayerFootLeft, TileLeft);
        float OverlapRight = FMath::Min(PlayerFootRight, TileRight);
        bool bOverFoot = (OverlapLeft < OverlapRight);

        if (bOverFoot && AttemptY + HalfSizeP.Y >= TileTop && Player.Velocity.Y >= 0.0f)
        {
            if (TileTop > BestTileTop)
            {
                BestTileTop = TileTop;
                bFoundGround = true;
            }
        }
    }

    if (bFoundGround)
    {
        Player.Position.Y = BestTileTop - HalfSizeP.Y;
        Player.Velocity.Y = 0;
        Player.bIsOnGround = true;
    }
    else
    {
        Player.Position.Y = AttemptY;
        Player.bIsOnGround = false;
    }

    if (Player.Position.Y > GroundY + 300.f)
    {
        GameOver();
        return;
    }
    if (Player.Position.X - HalfSizeP.X < 2.f)
    {
        GameOver();
        return;
    }
}

void UNeonRunnerMiniGame::CheckCollisions()
{
    FVector2D RunnerCenter = Player.Position;
    for (const FNeonRunnerObstacle& Obs : Obstacles)
    {
        if (!Obs.bIsActive)
            continue;

        float dx = FMath::Abs(RunnerCenter.X - Obs.Position.X);
        float dy = FMath::Abs(RunnerCenter.Y - Obs.Position.Y);
        float collideX = (Player.Size.X + Obs.Size.X) * 0.42f;
        float collideY = (Player.Size.Y + Obs.Size.Y) * 0.48f;

        if (dx < collideX && dy < collideY)
        {
            GameOver();
            return;
        }
    }
}

void UNeonRunnerMiniGame::GameOver()
{
    if (bIsGameOver) return;
    bIsGameOver = true;
    if (WidgetRef) WidgetRef->ShowGameOver();
    OnMiniGameEnded.Broadcast(false);
    EndGame();
}

void UNeonRunnerMiniGame::EndGame()
{
    if (OwningController)
    {
        if (UWorld* World = OwningController->GetWorld())
            World->GetTimerManager().ClearTimer(TickTimerHandle);

        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwningController->GetLocalPlayer()))
        {
            if (RunnerIMC) Subsystem->RemoveMappingContext(RunnerIMC);
            if (GameplayIMC) Subsystem->AddMappingContext(GameplayIMC, 0);
        }
    }
    if (WidgetRef)
    {
        WidgetRef->RemoveFromParent();
        WidgetRef = nullptr;
    }
    bIsGameOver = true;
    CleanupInput();
    Obstacles.Empty();
    Tiles.Empty();
    OwningController = nullptr;
}

void UNeonRunnerMiniGame::CreateWidget()
{
    if (!OwningController || !WidgetClass) return;
    WidgetRef = ::CreateWidget<UNeonRunnerWidget>(OwningController, WidgetClass);
    if (WidgetRef) WidgetRef->AddToViewport(0);
}

void UNeonRunnerMiniGame::SetupInput()
{
    if (!OwningController) return;
    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(OwningController->InputComponent);
    if (EnhancedInput)
    {
        EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &UNeonRunnerMiniGame::OnJump);
    }
}

void UNeonRunnerMiniGame::CleanupInput()
{
    // Optional
}

void UNeonRunnerMiniGame::OnJump()
{
    if (bIsGameOver) return;
    if (Player.bIsOnGround)
    {
        Player.Velocity.Y = JumpVelocity;
        Player.bIsOnGround = false;
    }
}

void UNeonRunnerMiniGame::UpdateWidget()
{
    if (!WidgetRef) return;
    WidgetRef->DrawGameObjects(Player, Obstacles, Tiles, bIsGameOver, SurvivalTime, VictoryTime);
}