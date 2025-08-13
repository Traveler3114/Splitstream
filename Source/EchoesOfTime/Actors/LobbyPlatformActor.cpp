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

// (Spawn/Destroy pawn stubs left commented out as before...)

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