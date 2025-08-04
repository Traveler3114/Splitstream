#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OpenFriendsListButton.generated.h"

class UButton;

UCLASS()
class ECHOESOFTIME_API UOpenFriendsListButton : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UButton* OpenFriendsList_btn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Friends")
	TSubclassOf<class UFriendList> FriendListWidgetClass;


	UFUNCTION()
	void OnFriendsListButtonClicked();
};