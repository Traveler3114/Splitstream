#include "DroneSpawner.h"
#include "Characters/DronePawn.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/PointActors/NavNode.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

// Sets default values
ADroneSpawner::ADroneSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	// Make PrinterMesh the root
	PrinterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrinterMesh"));
	RootComponent = PrinterMesh;

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	PlatformMesh->SetupAttachment(PrinterMesh);

	OpeningMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OpeningMesh"));
	OpeningMesh->SetupAttachment(PrinterMesh);

	CountdownText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("CountdownText"));
	CountdownText->SetupAttachment(PrinterMesh);
	CountdownText->SetText(FText::GetEmpty());
	CountdownText->SetHorizontalAlignment(EHTA_Center);
	CountdownText->SetVisibility(false);
}

// Called when the game starts or when spawned
void ADroneSpawner::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> FoundDrones;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADronePawn::StaticClass(), FoundDrones);
	for (AActor* Actor : FoundDrones)
	{
		ADronePawn* Drone = Cast<ADronePawn>(Actor);
		if (Drone)
		{
			BindToDroneDeath(Drone);
			SpawnedDrones.Add(Drone);
		}
	}
}

void ADroneSpawner::BindToDroneDeath(ADronePawn* Drone)
{
	if (Drone)
	{
		Drone->OnDroneDeath.RemoveDynamic(this, &ADroneSpawner::HandleDroneDeath);
		Drone->OnDroneDeath.AddDynamic(this, &ADroneSpawner::HandleDroneDeath);
	}
}

void ADroneSpawner::HandleDroneDeath(ADronePawn* DeadDrone)
{
	SpawnedDrones.Remove(DeadDrone);

	// Start the countdown for respawn
	RespawnTimeLeft = RespawnDelay;
	if (CountdownText)
	{
		CountdownText->SetText(FText::Format(NSLOCTEXT("DroneSpawner", "RespawnIn", "Spawning Drone in {0}"), FText::AsNumber((int32)RespawnTimeLeft)));
		CountdownText->SetVisibility(true);
	}

	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ADroneSpawner::OnRespawnTimerFinished, RespawnDelay, false);
	GetWorldTimerManager().SetTimer(TextUpdateTimerHandle, this, &ADroneSpawner::UpdateCountdownText, 1.0f, true);
}

void ADroneSpawner::UpdateCountdownText()
{
	RespawnTimeLeft -= 1.0f;
	if (RespawnTimeLeft > 0)
	{
		if (CountdownText)
		{
			CountdownText->SetText(FText::Format(NSLOCTEXT("DroneSpawner", "RespawnIn", "Spawning Drone in {0}"), FText::AsNumber((int32)RespawnTimeLeft)));
		}
	}
	else
	{
		// Will be hidden in OnRespawnTimerFinished
		GetWorldTimerManager().ClearTimer(TextUpdateTimerHandle);
	}
}

void ADroneSpawner::OnRespawnTimerFinished()
{
	SpawnDrone();

	// Hide the countdown text
	if (CountdownText)
	{
		CountdownText->SetVisibility(false);
	}
	RespawnTimeLeft = 0.0f;
	GetWorldTimerManager().ClearTimer(TextUpdateTimerHandle);
	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
}

void ADroneSpawner::SpawnDrone()
{
	FVector TempSpawnLocation = GetActorLocation();
	FRotator TempSpawnRotation = GetActorRotation();
	FActorSpawnParameters SpawnParams;
	ADronePawn* NewDrone = GetWorld()->SpawnActor<ADronePawn>(DroneClass, TempSpawnLocation, TempSpawnRotation, SpawnParams);
	NewDrone->TimelineEra = TimelineEra;

	if (!NewDrone)
		return;

	TArray<AActor*> FoundNodes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANavNode::StaticClass(), FoundNodes);

	TArray<ANavNode*> MatchingNodes;

	for (AActor* Actor : FoundNodes)
	{
		ANavNode* Node = Cast<ANavNode>(Actor);
		if (Node && Node->TimelineEra == NewDrone->TimelineEra && Node->NodeType == ENavNodeType::Sky)
		{
			MatchingNodes.Add(Node);
		}
	}

	BindToDroneDeath(NewDrone);
	SpawnedDrones.Add(NewDrone);

	if (MatchingNodes.Num() > 0)
	{
		ANavNode* SpawnNode = MatchingNodes[FMath::RandHelper(MatchingNodes.Num())];
		NewDrone->StartStateTreeAtNode(SpawnNode);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No NavNodes found for TimelineEra and NodeType that matches the drone! Drone will remain at spawn location."));
	}
}