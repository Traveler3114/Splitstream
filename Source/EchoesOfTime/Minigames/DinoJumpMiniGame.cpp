#include "DinoJumpMiniGame.h"
#include "Widgets/Minigames/DinoJumpWidget.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"

UDinoJumpMiniGame::UDinoJumpMiniGame()
	: GroundLevelY(0), TimeSinceLastObstacle(0.f), NextObsDistance(0.f), SurvivalTime(0.f),
	  bIsGameOver(false), bVictoryAchieved(false),
	  WidgetRef(nullptr), OwningController(nullptr)
{
}

FVector2D UDinoJumpMiniGame::GetPlayAreaSize() const
{
	if (WidgetRef && WidgetRef->GameCanvas)
		return WidgetRef->GameCanvas->GetCachedGeometry().GetLocalSize();
	return FVector2D(1920, 1080);
}

FVector2D UDinoJumpMiniGame::GetTextureSize(UTexture2D* Texture) const
{
	if (Texture)
		return FVector2D(Texture->GetImportedSize().X, Texture->GetImportedSize().Y);
	return FVector2D(96, 96);
}

void UDinoJumpMiniGame::StartGame(APlayerController* PlayerController)
{
	OwningController = PlayerController;
	bIsGameOver = false;
	bVictoryAchieved = false;
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
		World->GetTimerManager().SetTimerForNextTick(this, &UDinoJumpMiniGame::TickGame);

	SetupInput();
	UpdateWidget();
}

void UDinoJumpMiniGame::TickGame()
{
	if (bIsGameOver) return;

	float DeltaTime = 0.016f;
	SurvivalTime += DeltaTime;
	TimeSinceLastObstacle += ObstacleSpeed * DeltaTime;

	if (TimeSinceLastObstacle >= NextObsDistance)
	{
		SpawnObstacle();
		TimeSinceLastObstacle = 0.f;
		NextObsDistance = FMath::FRandRange(ObstacleMinDistance, ObstacleMaxDistance);
	}

	UpdatePlayer(DeltaTime);
	UpdateObstacles(DeltaTime);
	CheckCollisions();

	UpdateWidget();

	if (SurvivalTime >= VictoryTime && !bVictoryAchieved)
		Victory();

	// Keep ticking
	if (!bIsGameOver && OwningController && OwningController->GetWorld())
		OwningController->GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UDinoJumpMiniGame::TickGame);
}

void UDinoJumpMiniGame::UpdatePlayer(float DeltaTime)
{
	if (!Player.bIsOnGround)
	{
		Player.Velocity.Y += Gravity * DeltaTime;
		Player.Position.Y += Player.Velocity.Y * DeltaTime;
		if (Player.Position.Y >= GroundLevelY)
		{
			Player.Position.Y = GroundLevelY;
			Player.bIsOnGround = true;
			Player.Velocity.Y = 0.f;
		}
	}
}

void UDinoJumpMiniGame::UpdateObstacles(float DeltaTime)
{
	float speed = ObstacleSpeed * DeltaTime;
	for (FDinoObstacle& Obs : Obstacles)
	{
		Obs.Position.X -= speed;
		if (Obs.Position.X + Obs.Size.X*0.5f < 0.f)
			Obs.bIsActive = false;
	}
	Obstacles.RemoveAll([](const FDinoObstacle& Obs){ return !Obs.bIsActive; });
}

void UDinoJumpMiniGame::SpawnObstacle()
{
	FVector2D Area = GetPlayAreaSize();
	FDinoObstacle Obs;
	Obs.Texture = ObstacleTexture;
	Obs.Size = GetTextureSize(ObstacleTexture);
	float YOffset = FMath::FRandRange(-ObstacleSpawnRangeY, ObstacleSpawnRangeY);
	Obs.Position = FVector2D(Area.X + Obs.Size.X * 0.5f, GroundLevelY + YOffset);
	Obs.bIsActive = true;
	Obstacles.Add(Obs);
}

void UDinoJumpMiniGame::CheckCollisions()
{
	FVector2D DinoCenter = Player.Position;
	for (const FDinoObstacle& Obs : Obstacles)
	{
		if (!Obs.bIsActive) continue;
		FVector2D ObsCenter(Obs.Position.X, Obs.Position.Y);

		float dx = FMath::Abs(DinoCenter.X - ObsCenter.X);
		float dy = FMath::Abs(DinoCenter.Y - ObsCenter.Y);
		float collideX = (Player.Size.X + Obs.Size.X) * 0.45f;
		float collideY = (Player.Size.Y + Obs.Size.Y) * 0.4f;
		if (dx < collideX && dy < collideY)
		{
			GameOver();
			return;
		}
	}
}

void UDinoJumpMiniGame::GameOver()
{
	if (bIsGameOver) return;
	bIsGameOver = true;
	if (WidgetRef) WidgetRef->ShowGameOver();
	OnMiniGameEnded.Broadcast(false);
	EndGame();
}

void UDinoJumpMiniGame::Victory()
{
	if (bIsGameOver) return;
	bIsGameOver = true;
	bVictoryAchieved = true;
	if (WidgetRef) WidgetRef->ShowVictory();
	OnMiniGameEnded.Broadcast(true);
	EndGame();
}

void UDinoJumpMiniGame::EndGame()
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

void UDinoJumpMiniGame::CreateWidget()
{
	if (!OwningController || !WidgetClass) return;
	WidgetRef = ::CreateWidget<UDinoJumpWidget>(OwningController, WidgetClass);
	if (WidgetRef) WidgetRef->AddToViewport(0);
}

void UDinoJumpMiniGame::SetupInput()
{
	if (!OwningController) return;
	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(OwningController->InputComponent);
	if (EnhancedInput)
	{
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &UDinoJumpMiniGame::OnJump);
	}
}

void UDinoJumpMiniGame::CleanupInput()
{
	// Optional: Clean up input binds here if needed
}

void UDinoJumpMiniGame::OnJump()
{
	if (bIsGameOver) return;
	if (Player.bIsOnGround)
	{
		Player.Velocity.Y = JumpVelocity;
		Player.bIsOnGround = false;
	}
}

void UDinoJumpMiniGame::UpdateWidget()
{
	if (!WidgetRef) return;
	WidgetRef->DrawGameObjects(Player, Obstacles, bIsGameOver, SurvivalTime, VictoryTime);
}