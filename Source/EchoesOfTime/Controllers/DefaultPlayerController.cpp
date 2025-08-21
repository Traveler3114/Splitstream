// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultPlayerController.h"
#include "Widgets/HUD/CharacterHUD.h"
#include "Widgets/HUD/CharacterOverlay.h"

void ADefaultPlayerController::BeginPlay()
{
	Super::BeginPlay();
	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;
	if (CharacterHUD)
	{
		if (CharacterHUD->CharacterOverlay == nullptr) CharacterHUD->AddCharacterOverlay();
	}
}

