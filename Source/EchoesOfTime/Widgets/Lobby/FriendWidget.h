// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FriendWidget.generated.h"

/**
 * 
 */
UCLASS()
class ECHOESOFTIME_API UFriendWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:
	UPROPERTY(meta = (BindWidget))
	class UButton* invite_btn;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* playername_txt;

	UPROPERTY(meta = (BindWidget))
	class UImage* avatar_img;
};
