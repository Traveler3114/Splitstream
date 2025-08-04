#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EchoesOfTime/Actors/LobbyPlatformActor.h"
#include "FriendList.generated.h"

UCLASS()
class ECHOESOFTIME_API UFriendList : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY()
	ALobbyPlatformActor* LobbyActor;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// How much extra area (in pixels) beyond the widget bounds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FriendList")
	float ExtraArea = 20.0f;
};