// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerLobbyInfo.generated.h"


UCLASS()
class ECHOESOFTIME_API UPlayerLobbyInfo : public UUserWidget
{
	GENERATED_BODY()
	

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "PlayerLobbyInfo")
	class UButton* kick_btn;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "PlayerLobbyInfo")
	class UTextBlock* playername_txt;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "PlayerLobbyInfo")
	class UImage* avatar_img;

	UFUNCTION(BlueprintCallable, Category = "PlayerLobbyInfo")
	void SetPlayerName(const FText& Name);

	UFUNCTION(BlueprintCallable, Category = "PlayerLobbyInfo")
	void SetAvatarTexture(UTexture2D* Texture);

	UFUNCTION(BlueprintCallable, Category = "PlayerLobbyInfo")
	void SetKickButtonVisible(bool bVisible);
};
