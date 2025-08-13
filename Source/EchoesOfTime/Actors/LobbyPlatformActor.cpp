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
#include "Blueprint/UserWidget.h"

// Constructor
ALobbyPlatformActor::ALobbyPlatformActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;
	SetRootComponent(RootScene);

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

	// Initial UI state: show button, hide friend list
	SetFriendListVisible(false);

	// Bind to button widget delegate
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

	// Bind to friend list widget delegate
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

void ALobbyPlatformActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALobbyPlatformActor, OccupantPlayerState);
}

// Seat control
bool ALobbyPlatformActor::ServerAssignOccupant(APlayerState* NewOccupant)
{
	if (!HasAuthority())     return false;
	if (OccupantPlayerState) return false;
	if (!NewOccupant)        return false;

	OccupantPlayerState = NewOccupant;
	SpawnOccupantPawn();
	NotifyOccupantChanged();
	return true;
}

bool ALobbyPlatformActor::ServerClearOccupant()
{
	if (!HasAuthority())      return false;
	if (!OccupantPlayerState) return false;

	if (bDestroyPawnOnClear)
	{
		DestroyOccupantPawn();
	}

	OccupantPlayerState = nullptr;
	NotifyOccupantChanged();
	return true;
}

// Rep notify
void ALobbyPlatformActor::OnRep_OccupantPlayerState()
{
	NotifyOccupantChanged();
}

void ALobbyPlatformActor::NotifyOccupantChanged()
{
	UpdateWidgetsForOccupant();
	OnOccupantChanged.Broadcast(this, OccupantPlayerState);
}

// Pawn spawn/destroy
void ALobbyPlatformActor::SpawnOccupantPawn()
{
	//if (!HasAuthority())          return;
	//if (!OccupantPlayerState)     return;
	//if (!LobbyPawnClass)          return;

	//if (OccupantLobbyPawn && !OccupantLobbyPawn->IsPendingKill())
	//{
	//	DestroyOccupantPawn();
	//}

	//UWorld* World = GetWorld();
	//if (!World) return;

	//AController* Controller = Cast<AController>(OccupantPlayerState->GetOwner());

	//const FVector Loc = SpawnPoint ? SpawnPoint->GetComponentLocation() : GetActorLocation();
	//const FRotator Rot = SpawnPoint ? SpawnPoint->GetComponentRotation() : GetActorRotation();

	//FActorSpawnParameters Params;
	//Params.Owner = Controller;
	//Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	//APawn* NewPawn = World->SpawnActor<APawn>(LobbyPawnClass, Loc, Rot, Params);
	//if (NewPawn)
	//{
	//	OccupantLobbyPawn = NewPawn;
	//	if (Controller)
	//	{
	//		Controller->Possess(NewPawn);
	//	}
	//}
}

void ALobbyPlatformActor::DestroyOccupantPawn()
{
	//if (!HasAuthority()) return;
	//if (OccupantLobbyPawn && !OccupantLobbyPawn->IsPendingKill())
	//{
	//	OccupantLobbyPawn->Destroy();
	//}
	//OccupantLobbyPawn = nullptr;
}

// UI update based on occupant (expand later as needed)
void ALobbyPlatformActor::UpdateWidgetsForOccupant()
{
	// Example placeholder: you can push occupant data to PlayerLobbyInfo here.
	if (PlayerLobbyInfo)
	{
		if (UUserWidget* UW = PlayerLobbyInfo->GetUserWidgetObject())
		{
			// Implement a BlueprintImplementableEvent on that widget to receive occupant info if desired.
			// E.g., call UW->SetVisibility(...) or ProcessEvent on a custom function.
		}
	}
}

// Helper to toggle friend list vs button
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

// Delegate handlers
void ALobbyPlatformActor::HandleShowFriendListRequested()
{
	SetFriendListVisible(true);
}

void ALobbyPlatformActor::HandleShowOpenButtonRequested()
{
	SetFriendListVisible(false);
}