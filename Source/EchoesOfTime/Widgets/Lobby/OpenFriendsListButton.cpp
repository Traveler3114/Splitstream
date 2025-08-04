#include "OpenFriendsListButton.h"
#include "Components/Button.h"
#include "FriendList.h"
#include "Blueprint/UserWidget.h"
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
	if (UWorld* World = GetWorld())
	{
		// Use the Blueprint class if set, otherwise fallback to C++ class
		TSubclassOf<UFriendList> WidgetClass = FriendListWidgetClass;
		if (UFriendList* FriendListWidget = CreateWidget<UFriendList>(World, WidgetClass))
		{
			FriendListWidget->AddToViewport();
		}
	}
}