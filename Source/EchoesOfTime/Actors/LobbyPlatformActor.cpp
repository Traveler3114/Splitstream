#include "LobbyPlatformActor.h"

#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

#include "Widgets/Lobby/OpenFriendsListButton.h"
#include "Widgets/Lobby/FriendList.h"
#include "Widgets/Lobby/PlayerLobbyInfo.h"          // NEW
#include "DefaultPlayerState.h"                    // NEW
#include "Blueprint/UserWidget.h"

ALobbyPlatformActor::ALobbyPlatformActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	PlatformMesh->SetupAttachment(RootScene);
	PlatformMesh->SetCollisionProfileName(TEXT("BlockAll"));

	SpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnPoint"));
	SpawnPoint->SetupAttachment(RootScene);
	SpawnPoint->SetRelativeLocation(FVector(0.f, 0.f, 50.f));

	OpenFriendListButton = CreateDefaultSubobject<UWidgetComponent>(TEXT("OpenFriendListButton"));
	OpenFriendListButton->SetupAttachment(RootScene);

	FriendList = CreateDefaultSubobject<UWidgetComponent>(TEXT("FriendList"));
	FriendList->SetupAttachment(RootScene);

	PlayerLobbyInfo = CreateDefaultSubobject<UWidgetComponent>(TEXT("PlayerLobbyInfo"));
	PlayerLobbyInfo->SetupAttachment(RootScene);
}

void ALobbyPlatformActor::BeginPlay()
{
	Super::BeginPlay();

	SetFriendListVisible(false);

	if (OpenFriendListButton)
	{
		if (UUserWidget* UW = OpenFriendListButton->GetUserWidgetObject())
		{
			if (UOpenFriendsListButton* ButtonWidget = Cast<UOpenFriendsListButton>(UW))
			{
				ButtonWidget->OnShowFriendListRequested.AddDynamic(this, &ALobbyPlatformActor::HandleShowFriendListRequested);
			}
		}
	}

	if (FriendList)
	{
		if (UUserWidget* UW = FriendList->GetUserWidgetObject())
		{
			if (UFriendList* FriendListWidget = Cast<UFriendList>(UW))
			{
				FriendListWidget->OnShowButtonRequested.AddDynamic(this, &ALobbyPlatformActor::HandleShowOpenButtonRequested);
			}
		}
	}
}

void ALobbyPlatformActor::EndPlay(const EEndPlayReason::Type EndPlayReason) // NEW
{
	UnbindFromOccupantPlayerState();
	Super::EndPlay(EndPlayReason);
}

void ALobbyPlatformActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALobbyPlatformActor, OccupantPlayerState);
}

bool ALobbyPlatformActor::ServerAssignOccupant(APlayerState* NewOccupant)
{
	if (!HasAuthority() || OccupantPlayerState || !NewOccupant)
	{
		return false;
	}

	OccupantPlayerState = NewOccupant;
	SpawnOccupantPawn();
	NotifyOccupantChanged();
	return true;
}

bool ALobbyPlatformActor::ServerClearOccupant()
{
	if (!HasAuthority() || !OccupantPlayerState)
	{
		return false;
	}

	if (bDestroyPawnOnClear)
	{
		DestroyOccupantPawn();
	}

	OccupantPlayerState = nullptr;
	NotifyOccupantChanged();
	return true;
}

void ALobbyPlatformActor::OnRep_OccupantPlayerState()
{
	NotifyOccupantChanged();
}

void ALobbyPlatformActor::NotifyOccupantChanged()
{
	UnbindFromOccupantPlayerState();    // NEW
	BindToOccupantPlayerState();        // NEW

	UpdateWidgetsForOccupant();
	OnOccupantChanged.Broadcast(this, OccupantPlayerState);
}

void ALobbyPlatformActor::SpawnOccupantPawn()
{
	// Server only
	if (!HasAuthority())
	{
		return;
	}

	// Need a pawn class and an occupant
	if (!LobbyPawnClass || !OccupantPlayerState)
	{
		return;
	}

	// Clean up any existing display pawn
	if (IsValid(OccupantLobbyPawn))
	{
		OccupantLobbyPawn->Destroy();
		OccupantLobbyPawn = nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector SpawnLoc = SpawnPoint ? SpawnPoint->GetComponentLocation() : GetActorLocation();
	const FRotator SpawnRot = SpawnPoint ? SpawnPoint->GetComponentRotation() : GetActorRotation();

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APawn* NewPawn = World->SpawnActor<APawn>(LobbyPawnClass, SpawnLoc, SpawnRot, Params);
	if (!NewPawn)
	{
		return;
	}

	// Ensure it replicates so other clients see it (most Pawn classes already have bReplicates=true, but we enforce)
	NewPawn->SetReplicates(true);

	OccupantLobbyPawn = NewPawn;

	// Optional: possession (ONLY if you actually want the occupant to control / animate this pawn).
	// If this is just a static lobby display, you can skip this block.
	if (AController* Controller = Cast<AController>(OccupantPlayerState->GetOwner()))
	{
		// If you do NOT want to override their current pawn (e.g. they already have one),
		// remove or comment the possess call.
		Controller->Possess(NewPawn);
	}

	// OPTIONAL cosmetic adjustments:
	// NewPawn->SetActorEnableCollision(false);
	// NewPawn->DisableInput(nullptr);
}

void ALobbyPlatformActor::DestroyOccupantPawn()
{
	if (!HasAuthority())
	{
		return;
	}

	if (IsValid(OccupantLobbyPawn))
	{
		// If possessed, unpossess to avoid the controller trying to tick a destroyed pawn
		if (AController* Controller = OccupantLobbyPawn->GetController())
		{
			Controller->UnPossess();
		}

		OccupantLobbyPawn->Destroy();
		OccupantLobbyPawn = nullptr;
	}
}

void ALobbyPlatformActor::UpdateWidgetsForOccupant()
{
	if (!PlayerLobbyInfo)
		return;

	UUserWidget* UW = PlayerLobbyInfo->GetUserWidgetObject();
	if (!UW)
		return;

	if (UPlayerLobbyInfo* Info = Cast<UPlayerLobbyInfo>(UW))
	{
		if (OccupantPlayerState)
		{
			FString NameToShow = OccupantPlayerState->GetPlayerName();
			bool bReady = false;

			if (ADefaultPlayerState* DPS = Cast<ADefaultPlayerState>(OccupantPlayerState))
			{
				NameToShow = DPS->GetDisplayName();
				bReady = DPS->IsReady();
			}

			Info->SetPlayerName(FText::FromString(NameToShow));
			Info->SetReadyState(bReady);
		}
		else
		{
			Info->SetPlayerName(FText::FromString(TEXT("Empty")));
			Info->SetReadyState(false);
		}
	}
}

void ALobbyPlatformActor::SetFriendListVisible(bool bVisible)
{
	if (FriendList)
	{
		FriendList->SetVisibility(bVisible);
		if (UUserWidget* UW = FriendList->GetUserWidgetObject())
		{
			UW->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		}
	}
	if (OpenFriendListButton)
	{
		const bool bShowButton = !bVisible;
		OpenFriendListButton->SetVisibility(bShowButton);
		if (UUserWidget* UW = OpenFriendListButton->GetUserWidgetObject())
		{
			UW->SetVisibility(bShowButton ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		}
	}
}

void ALobbyPlatformActor::HandleShowFriendListRequested()
{
	SetFriendListVisible(true);
}

void ALobbyPlatformActor::HandleShowOpenButtonRequested()
{
	SetFriendListVisible(false);
}

// -------- NEW: Binding logic --------
void ALobbyPlatformActor::BindToOccupantPlayerState()
{
	if (!OccupantPlayerState) return;

	if (ADefaultPlayerState* DPS = Cast<ADefaultPlayerState>(OccupantPlayerState))
	{
		CachedDefaultPlayerState = DPS;
		DPS->OnPlayerMetaChanged.AddDynamic(this, &ALobbyPlatformActor::HandleOccupantMetaChanged);
		DPS->OnReadyChanged.AddDynamic(this, &ALobbyPlatformActor::HandleOccupantReadyChanged);
	}
}

void ALobbyPlatformActor::UnbindFromOccupantPlayerState()
{
	if (CachedDefaultPlayerState)
	{
		CachedDefaultPlayerState->OnPlayerMetaChanged.RemoveDynamic(this, &ALobbyPlatformActor::HandleOccupantMetaChanged);
		CachedDefaultPlayerState->OnReadyChanged.RemoveDynamic(this, &ALobbyPlatformActor::HandleOccupantReadyChanged);
		CachedDefaultPlayerState = nullptr;
	}
}

void ALobbyPlatformActor::HandleOccupantMetaChanged(ADefaultPlayerState* PS)
{
	UpdateWidgetsForOccupant();
}

void ALobbyPlatformActor::HandleOccupantReadyChanged(ADefaultPlayerState* PS)
{
	UpdateWidgetsForOccupant();
}
