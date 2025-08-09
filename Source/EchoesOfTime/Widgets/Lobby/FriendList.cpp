#include "FriendList.h"
#include "Kismet/GameplayStatics.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"

void UFriendList::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Get mouse position in viewport
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	float MouseX, MouseY;
	PC->GetMousePosition(MouseX, MouseY);

	// Get widget position and size in viewport
	FVector2D WidgetPos = MyGeometry.GetAbsolutePosition();
	FVector2D WidgetSize = MyGeometry.GetLocalSize();

	// Expand the area
	FVector2D Min = WidgetPos - FVector2D(ExtraArea, ExtraArea);
	FVector2D Max = WidgetPos + WidgetSize + FVector2D(ExtraArea, ExtraArea);

	// Check if mouse is outside the expanded area
	if (MouseX < Min.X || MouseX > Max.X || MouseY < Min.Y || MouseY > Max.Y)
	{
		OnShowButtonRequested.Broadcast();
	}
}


//void UFriendList::PopulateFriends()
//{
//	TArray<FBPFriendInfo> Friends;
//	UAdvancedSteamFriendsLibrary::GetSteamFriendsList(Friends);
//
//	for (const FBPFriendInfo& Friend : Friends)
//	{
//		UFriendWidget* FriendWidget = CreateWidget<UFriendWidget>(GetWorld(), FriendWidgetClass);
//		if (FriendWidget)
//		{
//			// Set friend name
//			FriendWidget->playeraname_txt->SetText(FText::FromString(Friend.DisplayName));
//
//			// Optionally set avatar and bind invite button here
//
//			// Add to your UI (e.g., a vertical box or scroll box)
//			FriendListContainer->AddChild(FriendWidget);
//		}
//	}
//}
