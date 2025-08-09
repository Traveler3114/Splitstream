#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FriendList.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShowButtonRequested);


UCLASS()
class ECHOESOFTIME_API UFriendList : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnShowButtonRequested OnShowButtonRequested;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* FriendListContainer;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
/*	void PopulateFriends()*/;

	// How much extra area (in pixels) beyond the widget bounds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FriendList")
	float ExtraArea = 20.0f;

	//TSubclassOf<class UFriendWidget> FriendWidgetClass;
};