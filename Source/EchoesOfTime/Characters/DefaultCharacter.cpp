#include "DefaultCharacter.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Widgets/Lobby/PlayerLobbyInfo.h"
#include "Net/UnrealNetwork.h"
#include "Components/ArrowComponent.h"

// Sets default values
ADefaultCharacter::ADefaultCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create Arrow Component
	ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
	ArrowComp->SetupAttachment(RootComponent);

	// Create Widget Component
	PlayerInfoWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PlayerInfoWidget"));
	PlayerInfoWidget->SetupAttachment(RootComponent);

	// Set widget properties to match Blueprint defaults
	PlayerInfoWidget->SetWidgetSpace(EWidgetSpace::Screen); // or EWidgetSpace::World if you want 3D
	PlayerInfoWidget->SetDrawSize(FVector2D(300.f, 100.f)); // Set to your desired size
	PlayerInfoWidget->SetRelativeLocation(FVector(0.f, 0.f, 120.f)); // Adjust Z as needed
	PlayerInfoWidget->SetPivot(FVector2D(0.5f, 0.5f)); // Centered
	PlayerInfoWidget->SetBlendMode(EWidgetBlendMode::Transparent); // Optional, for world widgets
	PlayerInfoWidget->SetTwoSided(true); // Optional, for world widgets
}

// Called when the game starts or when spawned
void ADefaultCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ADefaultCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ADefaultCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ADefaultCharacter::OnRep_PlayerInfo()
{
	if (PlayerInfoWidget)
	{
		PlayerInfoWidget->InitWidget();
		UUserWidget* UserWidget = PlayerInfoWidget->GetUserWidgetObject();
		if (UPlayerLobbyInfo* LobbyInfo = Cast<UPlayerLobbyInfo>(UserWidget))
		{
			LobbyInfo->SetPlayerName(FText::FromString(ReplicatedPlayerName));
			LobbyInfo->SetAvatarTexture(ReplicatedAvatarTexture);
		}
	}
}


void ADefaultCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADefaultCharacter, ReplicatedPlayerName);
	DOREPLIFETIME(ADefaultCharacter, ReplicatedAvatarTexture);
}