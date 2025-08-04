#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OpenFriendsListButton.generated.h"

class UButton;

UCLASS()
class ECHOESOFTIME_API UOpenFriendsListButton : public UUserWidget
{
	GENERATED_BODY()


public:
	UPROPERTY()
	class ALobbyPlatformActor* LobbyActor;
protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UButton* OpenFriendsList_btn;

	UFUNCTION()
	void OnFriendsListButtonClicked();

	// Add this to your UOpenFriendsListButton class

};