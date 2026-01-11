#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OpenFriendsListButton.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShowFriendListRequested);

UCLASS()
class SPLITSTREAM_API UOpenFriendsListButton : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnShowFriendListRequested OnShowFriendListRequested;

	UPROPERTY(meta = (BindWidget))
	UButton* OpenFriendsList_btn;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnFriendsListButtonClicked();
};



