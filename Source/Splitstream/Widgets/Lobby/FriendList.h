#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FriendList.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShowButtonRequested);


UCLASS()
class SPLITSTREAM_API UFriendList : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnShowButtonRequested OnShowButtonRequested;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "FriendList")
	class UVerticalBox* FriendListContainer;

protected:
	//virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// How much extra area (in pixels) beyond the widget bounds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FriendList")
	float ExtraArea = 20.0f;

	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
};