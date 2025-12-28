#include "NeonRunnerMiniGame.h"
#include "Widgets/Minigames/NeonRunnerWidget.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"

// ---- Tile segment state for extension in TickGame() ----
namespace {
    enum class ETileSegType { Flat, StairUp, StairDown };
    struct FTileGenState {
        ETileSegType currType = ETileSegType::Flat;
        int tilesLeft = 0;
        int stairStepsLeft = 0;
        float lastY = 0.f;
    };
    FTileGenState GTileGenExt;
}
// --------------------------------------------------------

UNeonRunnerMiniGame::UNeonRunnerMiniGame()
    : GroundLevelY(0), TimeSinceLastObstacle(0.f), NextObsDistance(0.f), SurvivalTime(0.f),
    bIsGameOver(false), bVictoryAchieved(false),
    WidgetRef(nullptr), OwningController(nullptr), bGravityFlipped(false)
{
    PlayerStartX = 960.f;
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

float UNeonRunnerMiniGame::GetCurrentGroundY() const
{
    return bGravityFlipped ? 150.f : GroundY;
}

static int RandomFlatLen() { return FMath::RandRange(1, 10); }
static int RandomStairSteps() { return FMath::RandRange(1, 5); }

void UNeonRunnerMiniGame::StartGame(APlayerController* PlayerController)
{
    OwningController = PlayerController;
    bIsGameOver = false;
    bVictoryAchieved = false;
    bGravityFlipped = false;
    SurvivalTime = 0.f;
    TimeSinceLastObstacle = 0.f;
    NextObsDistance = FMath::FRandRange(ObstacleMinDistance, ObstacleMaxDistance);

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwningController->GetLocalPlayer()))
    {
        if (GameplayIMC) Subsystem->RemoveMappingContext(GameplayIMC);
        if (DinoIMC) Subsystem->AddMappingContext(DinoIMC, 100);
    }

    FVector2D Area = GetPlayAreaSize();
    Player = FDinoPlayer();
    Player.Texture = DinoTexture;
    Player.Size = GetTextureSize(DinoTexture);
    Player.Velocity = FVector2D::ZeroVector;

    GroundLevelY = GroundY;
    Obstacles.Empty();

    Tiles.Empty();
    float y = GroundY + TileSize * 0.5f;
    float prevY = y;
    float minY = GroundY - 3 * TileSize;
    float maxY = GroundY + 3 * TileSize;

    const int InitialFlatTiles = 24;
    for (int i = 0; i < InitialFlatTiles; ++i) {
        FDinoObstacle floorTile;
        floorTile.Type = EObstacleType::Tile;
        floorTile.Texture = TileTexture;
        floorTile.Size = FVector2D(TileSize, TileSize);
        floorTile.Position.X = i * TileSize + TileSize * 0.5f;
        floorTile.Position.Y = y;
        floorTile.bIsActive = true;
        Tiles.Add(floorTile);
        prevY = y;
    }

    int i = InitialFlatTiles;
    int lastI = i;
    GTileGenExt.currType = ETileSegType::Flat;
    GTileGenExt.tilesLeft = 0;
    GTileGenExt.stairStepsLeft = 0;
    GTileGenExt.lastY = prevY;

    // Fill view + buffer
    float canvasW = Area.X;
    int NumInitialTiles = FMath::CeilToInt(canvasW / TileSize) + 24;

    while (i < NumInitialTiles)
    {
        // Start new segment if needed
        if (GTileGenExt.tilesLeft == 0) {
            int which = FMath::RandRange(0, 2);
            if (which == 0) {
                // Flat segment
                GTileGenExt.currType = ETileSegType::Flat;
                GTileGenExt.tilesLeft = RandomFlatLen();
                GTileGenExt.stairStepsLeft = 0;
            }
            else {
                // StairUp or StairDown
                GTileGenExt.currType = (FMath::RandBool() ? ETileSegType::StairUp : ETileSegType::StairDown);
                GTileGenExt.stairStepsLeft = RandomStairSteps();
                GTileGenExt.tilesLeft = GTileGenExt.stairStepsLeft * 2; // 2 tiles per step
            }
        }
        FDinoObstacle floorTile;
        floorTile.Type = EObstacleType::Tile;
        floorTile.Texture = TileTexture;
        floorTile.Size = FVector2D(TileSize, TileSize);
        floorTile.Position.X = i * TileSize + TileSize * 0.5f;

        if (GTileGenExt.currType == ETileSegType::Flat) {
            y = prevY;
        }
        else if ((GTileGenExt.tilesLeft % 2 == 0) && (GTileGenExt.stairStepsLeft > 0)) {
            float candidateY = prevY + (GTileGenExt.currType == ETileSegType::StairUp ? -TileSize : TileSize);
            // clamp
            if (candidateY < minY) candidateY = minY;
            if (candidateY > maxY) candidateY = maxY;
            y = candidateY;
            GTileGenExt.stairStepsLeft--;
        }
        else {
            y = prevY;
        }
        floorTile.Position.Y = y;
        floorTile.bIsActive = true;
        Tiles.Add(floorTile);
        prevY = y;
        GTileGenExt.lastY = y;
        GTileGenExt.tilesLeft--;
        i++;
    }

    // Prepare state for TickGame
    GTileGenExt.currType = ETileSegType::Flat;
    GTileGenExt.tilesLeft = 0;
    GTileGenExt.stairStepsLeft = 0;
    GTileGenExt.lastY = prevY;

    float StartX = PlayerStartX;
    if (Tiles.Num() > 0) {
        float BestDist = FLT_MAX;
        int BestIdx = 0;
        for (int i2 = 0; i2 < Tiles.Num(); ++i2) {
            float dist = FMath::Abs(Tiles[i2].Position.X - StartX);
            if (dist < BestDist) { BestDist = dist; BestIdx = i2; }
        }
        const FDinoObstacle& StartTile = Tiles[BestIdx];
        FVector2D HalfSizeT = StartTile.Size * 0.5f;
        FVector2D HalfSizeP = Player.Size * 0.5f;
        Player.Position.X = StartX;
        Player.Position.Y = StartTile.Position.Y - HalfSizeT.Y - HalfSizeP.Y;
    }
    else {
        Player.Position = FVector2D(StartX, GroundY);
    }
    Player.bIsOnGround = true;
    Player.Velocity = FVector2D::ZeroVector;

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

    float totalScroll = PlayerMoveSpeed * DeltaTime;
    for (FDinoObstacle& Tile : Tiles) {
        Tile.Position.X -= totalScroll;
        if (Tile.Position.X + Tile.Size.X * 0.5f < -100.f)
            Tile.bIsActive = false;
    }
    Tiles.RemoveAll([](const FDinoObstacle& T) { return !T.bIsActive; });

    // EXTREME random floor/stair
    FVector2D Area = GetPlayAreaSize();
    float MostRight = 0.f;
    for (const FDinoObstacle& Tile : Tiles)
        if (Tile.Position.X > MostRight)
            MostRight = Tile.Position.X;
    float minY = GroundY - 3 * TileSize;
    float maxY = GroundY + 3 * TileSize;

    while (MostRight < Area.X + TileSize * 2) {
        if (GTileGenExt.tilesLeft == 0) {
            int which = FMath::RandRange(0, 2);
            if (which == 0) {
                GTileGenExt.currType = ETileSegType::Flat;
                GTileGenExt.tilesLeft = RandomFlatLen();
                GTileGenExt.stairStepsLeft = 0;
            }
            else {
                GTileGenExt.currType = (FMath::RandBool() ? ETileSegType::StairUp : ETileSegType::StairDown);
                GTileGenExt.stairStepsLeft = RandomStairSteps();
                GTileGenExt.tilesLeft = GTileGenExt.stairStepsLeft * 2;
            }
        }

        FDinoObstacle floorTile;
        floorTile.Type = EObstacleType::Tile;
        floorTile.Texture = TileTexture;
        floorTile.Size = FVector2D(TileSize, TileSize);
        floorTile.Position.X = MostRight + TileSize;

        float y = GTileGenExt.lastY;
        if (GTileGenExt.currType == ETileSegType::Flat) {
            // nothing, stay level
        }
        else if ((GTileGenExt.tilesLeft % 2 == 0) && (GTileGenExt.stairStepsLeft > 0)) {
            float candidateY = GTileGenExt.lastY + (GTileGenExt.currType == ETileSegType::StairUp ? -TileSize : TileSize);
            // clamp
            if (candidateY < minY) candidateY = minY;
            if (candidateY > maxY) candidateY = maxY;
            y = candidateY;
            GTileGenExt.stairStepsLeft--;
        }
        floorTile.Position.Y = y;
        floorTile.bIsActive = true;
        Tiles.Add(floorTile);
        GTileGenExt.tilesLeft--;
        GTileGenExt.lastY = y;
        MostRight = floorTile.Position.X;
    }

    // Scroll obstacles
    for (FDinoObstacle& Obs : Obstacles)
    {
        Obs.Position.X -= PlayerMoveSpeed * DeltaTime;
        if (Obs.Type == EObstacleType::Gap)
            continue;
        if (Obs.Position.X + Obs.Size.X < 0.f)
            Obs.bIsActive = false;
    }
    Obstacles.RemoveAll([](const FDinoObstacle& Obs) { return !Obs.bIsActive; });

    TimeSinceLastObstacle += PlayerMoveSpeed * DeltaTime;
    if (TimeSinceLastObstacle >= NextObsDistance)
    {
        SpawnRandomObstacleChunk();
        TimeSinceLastObstacle = 0.f;
        NextObsDistance = FMath::FRandRange(ObstacleMinDistance, ObstacleMaxDistance);
    }

    UpdatePlayer(DeltaTime);
    CheckCollisions();
    UpdateWidget();

    if (SurvivalTime >= VictoryTime && !bVictoryAchieved)
        Victory();

    if (!bIsGameOver && OwningController && OwningController->GetWorld())
        OwningController->GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UNeonRunnerMiniGame::TickGame);
}

// --- Rest of your code below unchanged ---
void UNeonRunnerMiniGame::SpawnRandomObstacleChunk()
{
    FVector2D Area = GetPlayAreaSize();
    float groundY = GetCurrentGroundY();
    int Pattern = FMath::RandRange(0, 6);
    float baseX = Area.X + 80.0f;

    if (FMath::FRand() < PlatformGapChance)
    {
        FDinoObstacle gap;
        gap.Type = EObstacleType::Gap;
        gap.Size = FVector2D(200.f + FMath::FRandRange(-65.f, 110.f), 40.f);
        gap.Position = FVector2D(baseX + gap.Size.X * 0.5f, groundY + 300.f);
        gap.Texture = nullptr;
        gap.bIsActive = true;
        Obstacles.Add(gap);
        return;
    }
    if (Pattern <= 1)
    {
        FDinoObstacle spike;
        spike.Type = EObstacleType::Spike;
        spike.Texture = ObstacleTexture_Spike;
        spike.Size = FVector2D(52.f + FMath::FRandRange(-7.f, 11.f), 90.f + FMath::FRandRange(-18.f, 12.f));
        spike.Position = FVector2D(baseX, groundY);
        spike.bIsActive = true;
        Obstacles.Add(spike);
    }
    else if (Pattern == 2)
    {
        for (int i = 0; i < 2; ++i)
        {
            FDinoObstacle spike;
            spike.Type = EObstacleType::Spike;
            spike.Texture = ObstacleTexture_Spike;
            spike.Size = FVector2D(50.f, 100.f);
            spike.Position = FVector2D(baseX + i * 52.f, groundY);
            spike.bIsActive = true;
            Obstacles.Add(spike);
        }
    }
    else if (Pattern == 3)
    {
        FDinoObstacle block;
        block.Type = EObstacleType::Block;
        block.Texture = ObstacleTexture_Block;
        block.Size = FVector2D(60.f, 110.f + FMath::FRandRange(-22.f, 50.f));
        block.Position = FVector2D(baseX, groundY - 30.f);
        block.bIsActive = true;
        Obstacles.Add(block);
    }
    else if (Pattern == 4)
    {
        FDinoObstacle fly;
        fly.Type = EObstacleType::Flying;
        fly.Texture = ObstacleTexture_Flying;
        fly.Size = FVector2D(84.f, 40.f + FMath::FRandRange(-10.f, 10.f));
        fly.Position = FVector2D(baseX, groundY - 170.f - FMath::FRandRange(0.f, 100.f));
        fly.bIsActive = true;
        Obstacles.Add(fly);
    }
    else if (Pattern == 5)
    {
        FDinoObstacle portal;
        portal.Type = EObstacleType::Teleporter;
        portal.Texture = ObstacleTexture_Teleporter;
        portal.Size = FVector2D(60.f, 96.f);
        portal.Position = FVector2D(baseX, groundY);
        portal.bIsActive = true;
        Obstacles.Add(portal);
    }
    else
    {
        for (int i = 0; i < 3; ++i)
        {
            FDinoObstacle spike;
            spike.Type = EObstacleType::Spike;
            spike.Texture = ObstacleTexture_Spike;
            spike.Size = FVector2D(44.f, 85.f);
            spike.Position = FVector2D(baseX + i * 40.f, groundY);
            spike.bIsActive = true;
            Obstacles.Add(spike);
        }
    }
}

void UNeonRunnerMiniGame::UpdatePlayer(float DeltaTime)
{
    static bool bWasPushedPrev = false;

    FVector2D HalfSizeP = Player.Size * 0.5f;
    float gravityDir = (bGravityFlipped ? -1.f : 1.f);

    if (!Player.bIsOnGround)
        Player.Velocity.Y += Gravity * DeltaTime * gravityDir;
    float AttemptY = Player.Position.Y + Player.Velocity.Y * DeltaTime;

    float totalScroll = PlayerMoveSpeed * DeltaTime;
    float PlayerLeft = Player.Position.X - HalfSizeP.X;
    float PlayerRight = Player.Position.X + HalfSizeP.X;
    float PlayerTop = AttemptY - HalfSizeP.Y + 2.f;
    float PlayerBottom = AttemptY + HalfSizeP.Y - 2.f;

    bool bPushedThisFrame = false;

    for (const FDinoObstacle& Tile : Tiles)
    {
        if (!Tile.bIsActive) continue;
        FVector2D HalfSizeT = Tile.Size * 0.5f;
        float TileLeft = Tile.Position.X - HalfSizeT.X;
        float TileRight = Tile.Position.X + HalfSizeT.X;
        float TileTop = Tile.Position.Y - HalfSizeT.Y;
        float TileBottom = Tile.Position.Y + HalfSizeT.Y;
        bool bVerticalOverlap = (PlayerBottom > TileTop + 1.f && PlayerTop < TileBottom - 1.f);

        if (PlayerRight > TileLeft && PlayerLeft < TileLeft && bVerticalOverlap)
        {
            bPushedThisFrame = true;
            break;
        }
    }

    if (bPushedThisFrame)
    {
        Player.Position.X -= totalScroll;
        if (Player.Position.X > PlayerStartX) Player.Position.X = PlayerStartX;
    }

    float BestTileTop = FLT_MAX;
    float BestTileBottom = -FLT_MAX;
    bool bFoundFloor = false;

    float FootLeftX = Player.Position.X - HalfSizeP.X * 0.7f;
    float FootRightX = Player.Position.X + HalfSizeP.X * 0.7f;

    for (const FDinoObstacle& Tile : Tiles)
    {
        if (!Tile.bIsActive) continue;
        FVector2D HalfSizeT = Tile.Size * 0.5f;
        float TileLeft = Tile.Position.X - HalfSizeT.X;
        float TileRight = Tile.Position.X + HalfSizeT.X;
        float TileTop = Tile.Position.Y - HalfSizeT.Y;
        float TileBottom = Tile.Position.Y + HalfSizeT.Y;

        float OverlapLeft = FMath::Max(FootLeftX, TileLeft);
        float OverlapRight = FMath::Min(FootRightX, TileRight);
        bool bStandingOn = (OverlapLeft < OverlapRight);
        if (bStandingOn)
        {
            if (!bGravityFlipped)
            {
                if (Player.Position.Y + HalfSizeP.Y <= TileTop + 1.0f &&
                    AttemptY + HalfSizeP.Y >= TileTop - 1.0f)
                {
                    if (TileTop < BestTileTop)
                    {
                        BestTileTop = TileTop;
                        bFoundFloor = true;
                    }
                }
            }
            else
            {
                if (Player.Position.Y - HalfSizeP.Y >= TileBottom - 1.0f &&
                    AttemptY - HalfSizeP.Y <= TileBottom + 1.0f)
                {
                    if (TileBottom > BestTileBottom)
                    {
                        BestTileBottom = TileBottom;
                        bFoundFloor = true;
                    }
                }
            }
        }
    }

    if (bFoundFloor)
    {
        if (!bGravityFlipped)
        {
            Player.Position.Y = BestTileTop - HalfSizeP.Y;
            Player.Velocity.Y = 0;
            Player.bIsOnGround = true;
        }
        else
        {
            Player.Position.Y = BestTileBottom + HalfSizeP.Y;
            Player.Velocity.Y = 0;
            Player.bIsOnGround = true;
        }
    }
    else
    {
        Player.Position.Y = AttemptY;
        Player.bIsOnGround = false;
    }

    if (Player.Position.X - HalfSizeP.X < 2.f)
    {
        GameOver();
        return;
    }
    if ((!bGravityFlipped && Player.Position.Y > GroundY + 500.f) ||
        (bGravityFlipped && Player.Position.Y < 50.f))
    {
        GameOver();
        return;
    }

    bWasPushedPrev = bPushedThisFrame;
}

void UNeonRunnerMiniGame::UpdateObstacles(float DeltaTime)
{
    // NO LONGER NEEDED - see TickGame
}

void UNeonRunnerMiniGame::CheckCollisions()
{
    FVector2D DinoCenter = Player.Position;
    for (FDinoObstacle& Obs : Obstacles)
    {
        if (!Obs.bIsActive)
            continue;

        if (Obs.Type == EObstacleType::Gap)
        {
            float gapLeft = Obs.Position.X - Obs.Size.X * 0.5f;
            float gapRight = Obs.Position.X + Obs.Size.X * 0.5f;
            if (DinoCenter.X > gapLeft && DinoCenter.X < gapRight && Player.Position.Y == GetCurrentGroundY())
            {
                Player.bIsOnGround = false;
            }
            continue;
        }

        if (Obs.Type == EObstacleType::Teleporter)
        {
            float dx = FMath::Abs(Player.Position.X - Obs.Position.X);
            float dy = FMath::Abs(Player.Position.Y - Obs.Position.Y);
            float collideX = (Player.Size.X + Obs.Size.X) * 0.49f;
            float collideY = (Player.Size.Y + Obs.Size.Y) * 0.62f;
            if (dx < collideX && dy < collideY)
            {
                bGravityFlipped = !bGravityFlipped;
                Player.Velocity.Y = 0.f;
                Player.Position.Y = GetCurrentGroundY();
                Player.bIsOnGround = true;
                Obs.bIsActive = false;
                continue;
            }
            continue;
        }

        float dx = FMath::Abs(DinoCenter.X - Obs.Position.X);
        float dy = FMath::Abs(DinoCenter.Y - Obs.Position.Y);
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

void UNeonRunnerMiniGame::Victory()
{
    if (bIsGameOver) return;
    bIsGameOver = true;
    bVictoryAchieved = true;
    if (WidgetRef) WidgetRef->ShowVictory();
    OnMiniGameEnded.Broadcast(true);
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
            if (DinoIMC) Subsystem->RemoveMappingContext(DinoIMC);
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
        Player.Velocity.Y = bGravityFlipped ? -JumpVelocity : JumpVelocity;
        Player.bIsOnGround = false;
    }
}

void UNeonRunnerMiniGame::UpdateWidget()
{
    if (!WidgetRef) return;
    WidgetRef->DrawGameObjects(Player, Obstacles, Tiles, bIsGameOver, SurvivalTime, VictoryTime);
}