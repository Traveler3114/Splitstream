// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FriendWidget.generated.h"

/**
 * 
 */
UCLASS()
class SPLITSTREAM_API UFriendWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "FriendWidget")
	class UButton* invite_btn;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "FriendWidget")
	class UTextBlock* playername_txt;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "FriendWidget")
	class UImage* avatar_img;
};
