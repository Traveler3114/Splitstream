#include "OpenFriendsListButton.h"
#include "Components/Button.h"
#include "FriendList.h"
#include "Blueprint/UserWidget.h"
#include "EchoesOfTime/Actors/LobbyPlatformActor.h"
#include "Kismet/GameplayStatics.h"

void UOpenFriendsListButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (OpenFriendsList_btn)
	{
		OpenFriendsList_btn->OnClicked.AddDynamic(this, &UOpenFriendsListButton::OnFriendsListButtonClicked);
	}
}

void UOpenFriendsListButton::OnFriendsListButtonClicked()
{
	if (LobbyActor)
	{
		LobbyActor->ShowFriendList();
	}
}