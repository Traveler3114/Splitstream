#include "NeonRunnerMiniGame.h"
#include "Widgets/Minigames/NeonRunnerWidget.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"

UNeonRunnerMiniGame::UNeonRunnerMiniGame()
    : GroundLevelY(0), TimeSinceLastObstacle(0.f), NextObsDistance(0.f), SurvivalTime(0.f),
    bIsGameOver(false), bVictoryAchieved(false),
    WidgetRef(nullptr), OwningController(nullptr), bGravityFlipped(false)
{
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
    // Ceiling ground for gravity flip: use about 150 units from top
    return bGravityFlipped ? 150.f : GroundY;
}

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
    Player.Position = FVector2D(DinoStartX, GroundY);
    Player.bIsOnGround = true;
    Player.Velocity = FVector2D::ZeroVector;

    GroundLevelY = GroundY;
    Obstacles.Empty();

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

    // Scroll obstacles left to simulate dino running forward
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

// ----- Randomized Chunks: Spikes, Blocks, Flying, Gap/Pit, Portal (Teleporter) -----

void UNeonRunnerMiniGame::SpawnRandomObstacleChunk()
{
    FVector2D Area = GetPlayAreaSize();
    float groundY = GetCurrentGroundY();
    int Pattern = FMath::RandRange(0, 6);
    float baseX = Area.X + 80.0f;

    if (FMath::FRand() < PlatformGapChance)
    {
        // Pit/gap
        FDinoObstacle gap;
        gap.Type = EObstacleType::Gap;
        gap.Size = FVector2D(200.f + FMath::FRandRange(-65.f, 110.f), 40.f);
        gap.Position = FVector2D(baseX + gap.Size.X * 0.5f, groundY + 300.f); // Move far down (invisible)
        gap.Texture = nullptr;
        gap.bIsActive = true;
        Obstacles.Add(gap);
        return;
    }
    if (Pattern <= 1)
    {
        // Single spike
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
        // Two spikes
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
        // Block (jump wall)
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
        // Flying obstacle
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
        // Gravity flip teleporter!
        FDinoObstacle portal;
        portal.Type = EObstacleType::Teleporter;
        portal.Texture = ObstacleTexture_Teleporter;
        portal.Size = FVector2D(60.f, 96.f);
        // Place on ground or ceiling depending on current state
        portal.Position = FVector2D(baseX, groundY);
        portal.bIsActive = true;
        Obstacles.Add(portal);
    }
    // else: triple spike
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
    // Only vertical (jump, gravity, and death check)
    if (!Player.bIsOnGround)
    {
        float gravityDir = (bGravityFlipped ? -1.f : 1.f);
        Player.Velocity.Y += Gravity * DeltaTime * gravityDir;
        Player.Position.Y += Player.Velocity.Y * DeltaTime;

        float groundY = GetCurrentGroundY();
        // Die if out of bounds
        if ((!bGravityFlipped && Player.Position.Y > groundY + 130.f) ||
            (bGravityFlipped && Player.Position.Y < groundY - 130.f))
        {
            GameOver();
            return;
        }
        // Landed
        if ((!bGravityFlipped && Player.Position.Y >= groundY) ||
            (bGravityFlipped && Player.Position.Y <= groundY))
        {
            Player.Position.Y = groundY;
            Player.bIsOnGround = true;
            Player.Velocity.Y = 0.f;
        }
    }
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

        // Gaps: If player over a pit (and on ground), fall!
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

        // Teleporter: flip on overlap, remove portal
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

        // Spike/Block/Flying - bounding box collision
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
    WidgetRef->DrawGameObjects(Player, Obstacles, bIsGameOver, SurvivalTime, VictoryTime);
}