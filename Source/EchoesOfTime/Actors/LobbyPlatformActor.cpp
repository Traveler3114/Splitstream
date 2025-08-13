// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlatformActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/ArrowComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

// Sets default values
ALobbyPlatformActor::ALobbyPlatformActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	SetRootComponent(PlatformMesh);
	PlatformMesh->SetCollisionProfileName(TEXT("BlockAll"));

	SpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnPoint"));
	SpawnPoint->SetupAttachment(PlatformMesh);
	SpawnPoint->SetRelativeLocation(FVector(0.f, 0.f, 50.f));

	OpenFriendListButton = CreateDefaultSubobject<UWidgetComponent>(TEXT("OpenFriendListButton"));
	OpenFriendListButton->SetupAttachment(PlatformMesh);

	FriendList = CreateDefaultSubobject<UWidgetComponent>(TEXT("FriendList"));
	FriendList->SetupAttachment(PlatformMesh);

	PlayerLobbyInfo = CreateDefaultSubobject<UWidgetComponent>(TEXT("PlayerLobbyInfo"));
	PlayerLobbyInfo->SetupAttachment(PlatformMesh);
}

// Called when the game starts or when spawned
void ALobbyPlatformActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ALobbyPlatformActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}


void ALobbyPlatformActor::OnRep_OccupantPlayerState()
{

}	

