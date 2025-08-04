#include "LobbyPlatformActor.h"
#include "Components/SceneComponent.h"
#include "EchoesOfTime/Widgets/Lobby/OpenFriendsListButton.h"
#include "EchoesOfTime/Widgets/Lobby/FriendList.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"

ALobbyPlatformActor::ALobbyPlatformActor()
{
    RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootScene;

    PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
    PlatformMesh->SetupAttachment(RootComponent);

    OpenFriendsListButtonWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OpenFriendsListButtonWidget"));
    OpenFriendsListButtonWidget->SetupAttachment(RootComponent);
    OpenFriendsListButtonWidget->SetWidgetSpace(EWidgetSpace::Screen); // Or World, as needed

    FriendListWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("FriendListWidget"));
    FriendListWidget->SetupAttachment(RootComponent);
    FriendListWidget->SetWidgetSpace(EWidgetSpace::Screen); // Or World, as needed
    // Set the widget class in Blueprint or here if you have a reference
    OpenFriendsListButtonWidget->SetVisibility(true);
    FriendListWidget->SetVisibility(false);


}

void ALobbyPlatformActor::BeginPlay()
{
	Super::BeginPlay();

	// Set reference for button widget
	if (UUserWidget* Widget = Cast<UUserWidget>(OpenFriendsListButtonWidget->GetUserWidgetObject()))
	{
		if (UOpenFriendsListButton* ButtonWidget = Cast<UOpenFriendsListButton>(Widget))
		{
			ButtonWidget->LobbyActor = this;
		}
	}

	// Set reference for friend list widget
	if (UUserWidget* Widget = Cast<UUserWidget>(FriendListWidget->GetUserWidgetObject()))
	{
		if (UFriendList* FriendList = Cast<UFriendList>(Widget))
		{
			FriendList->LobbyActor = this;
		}
	}
}

void ALobbyPlatformActor::ShowFriendList()
{
    if (OpenFriendsListButtonWidget) OpenFriendsListButtonWidget->SetVisibility(false);
    if (FriendListWidget) FriendListWidget->SetVisibility(true);
}

void ALobbyPlatformActor::ShowButton()
{
    if (OpenFriendsListButtonWidget) OpenFriendsListButtonWidget->SetVisibility(true);
    if (FriendListWidget) FriendListWidget->SetVisibility(false);
}