#include "FriendList.h"
#include "Kismet/GameplayStatics.h"

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
		if (LobbyActor)
		{
			LobbyActor->ShowButton();
		}
	}
}