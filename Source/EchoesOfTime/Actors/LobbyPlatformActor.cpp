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
#include "Widgets/Lobby/PlayerLobbyInfo.h"
#include "DefaultPlayerState.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"

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

	// Start with friend list hidden (also sets button visibility)
	SetFriendListVisible(false);

	// Force early construction attempt for player info (optional)
	if (PlayerLobbyInfo)
	{
		PlayerLobbyInfo->GetUserWidgetObject();
		// Hide it initially (unoccupied)
		PlayerLobbyInfo->SetVisibility(false);
		if (UUserWidget* InfoUW = PlayerLobbyInfo->GetUserWidgetObject())
		{
			InfoUW->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

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

	// Cover construction race
	SchedulePlayerInfoRetry();
	UpdateOpenFriendButtonVisibility();
}

void ALobbyPlatformActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
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
	SchedulePlayerInfoRetry();
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
	SchedulePlayerInfoRetry();
	return true;
}

void ALobbyPlatformActor::OnRep_OccupantPlayerState()
{
	NotifyOccupantChanged();
	SchedulePlayerInfoRetry();
}

void ALobbyPlatformActor::NotifyOccupantChanged()
{
	UnbindFromOccupantPlayerState();
	BindToOccupantPlayerState();

	// Optional: auto-close friend list when occupied
	if (OccupantPlayerState && FriendList && FriendList->IsVisible())
	{
		SetFriendListVisible(false);
	}

	UpdateWidgetsForOccupant();
	UpdateOpenFriendButtonVisibility();

	OnOccupantChanged.Broadcast(this, OccupantPlayerState);
}

void ALobbyPlatformActor::SpawnOccupantPawn()
{
	if (!HasAuthority()) return;
	if (!LobbyPawnClass || !OccupantPlayerState) return;

	if (IsValid(OccupantLobbyPawn))
	{
		OccupantLobbyPawn->Destroy();
		OccupantLobbyPawn = nullptr;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	const FVector SpawnLoc = SpawnPoint ? SpawnPoint->GetComponentLocation() : GetActorLocation();
	const FRotator SpawnRot = SpawnPoint ? SpawnPoint->GetComponentRotation() : GetActorRotation();

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APawn* NewPawn = World->SpawnActor<APawn>(LobbyPawnClass, SpawnLoc, SpawnRot, Params);
	if (!NewPawn) return;

	NewPawn->SetReplicates(true);
	OccupantLobbyPawn = NewPawn;
}

void ALobbyPlatformActor::DestroyOccupantPawn()
{
	if (!HasAuthority()) return;

	if (IsValid(OccupantLobbyPawn))
	{
		OccupantLobbyPawn->Destroy();
		OccupantLobbyPawn = nullptr;
	}
}

void ALobbyPlatformActor::UpdateWidgetsForOccupant()
{
	if (!PlayerLobbyInfo) return;

	UUserWidget* UW = PlayerLobbyInfo->GetUserWidgetObject();
	if (!UW)
	{
		SchedulePlayerInfoRetry();
		return;
	}

	// Show/hide PlayerLobbyInfo component based on occupancy.
	if (!OccupantPlayerState)
	{
		// Hide when empty
		PlayerLobbyInfo->SetVisibility(false);
		if (UUserWidget* InfoUW = PlayerLobbyInfo->GetUserWidgetObject())
		{
			InfoUW->SetVisibility(ESlateVisibility::Collapsed);
			if (UPlayerLobbyInfo* Info = Cast<UPlayerLobbyInfo>(InfoUW))
			{
				// Clear any previous target to be safe
				Info->SetTargetPlayerState(nullptr);
				Info->SetKickButtonVisible(false);
			}
		}
		return;
	}

	// Ensure visible when occupied
	PlayerLobbyInfo->SetVisibility(true);
	if (UUserWidget* InfoUW = PlayerLobbyInfo->GetUserWidgetObject())
	{
		InfoUW->SetVisibility(ESlateVisibility::Visible);
	}

	if (UPlayerLobbyInfo* Info = Cast<UPlayerLobbyInfo>(UW))
	{
		// Fill name/ready
		FString NameToShow = OccupantPlayerState->GetPlayerName();
		bool bReady = false;

		if (ADefaultPlayerState* DPS = Cast<ADefaultPlayerState>(OccupantPlayerState))
		{
			NameToShow = DPS->GetDisplayName();
			bReady = DPS->IsReady();
		}

		Info->SetPlayerName(FText::FromString(NameToShow));
		Info->SetReadyState(bReady);

		// Kick button visibility: only host should see it, and never on their own slot
		bool bShowKick = false;
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* LocalPC = World->GetFirstPlayerController())
			{
				const bool bIsHostLocal = LocalPC->HasAuthority();
				const bool bIsSelfSlot = (LocalPC->PlayerState == OccupantPlayerState);
				bShowKick = bIsHostLocal && !bIsSelfSlot;
			}
		}
		Info->SetKickButtonVisible(bShowKick);

		// CRITICAL: set the target PlayerState so the kick RPC knows whom to kick
		Info->SetTargetPlayerState(OccupantPlayerState);
	}
}

void ALobbyPlatformActor::SchedulePlayerInfoRetry()
{
	if (bPendingPlayerInfoRetry) return;
	if (!GetWorld()) return;

	bPendingPlayerInfoRetry = true;
	FTimerHandle Tmp;
	GetWorld()->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateUObject(this, &ALobbyPlatformActor::RetryUpdatePlayerInfo));
}

void ALobbyPlatformActor::RetryUpdatePlayerInfo()
{
	bPendingPlayerInfoRetry = false;
	UpdateWidgetsForOccupant();
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
	UpdateOpenFriendButtonVisibility();
}

void ALobbyPlatformActor::HandleShowFriendListRequested()
{
	SetFriendListVisible(true);
}

void ALobbyPlatformActor::HandleShowOpenButtonRequested()
{
	SetFriendListVisible(false);
}

void ALobbyPlatformActor::UpdateOpenFriendButtonVisibility()
{
	if (!OpenFriendListButton) return;

	const bool bFriendListVisible = FriendList && FriendList->IsVisible();
	const bool bUnoccupied = (OccupantPlayerState == nullptr);
	const bool bShowButton = bUnoccupied && !bFriendListVisible;

	OpenFriendListButton->SetVisibility(bShowButton);
	if (UUserWidget* UW = OpenFriendListButton->GetUserWidgetObject())
	{
		UW->SetVisibility(bShowButton ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void ALobbyPlatformActor::BindToOccupantPlayerState()
{
	CachedDefaultPlayerState = nullptr;
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

void ALobbyPlatformActor::HandleOccupantMetaChanged(ADefaultPlayerState* /*PS*/)
{
	UpdateWidgetsForOccupant();
}

void ALobbyPlatformActor::HandleOccupantReadyChanged(ADefaultPlayerState* /*PS*/)
{
	UpdateWidgetsForOccupant();
}