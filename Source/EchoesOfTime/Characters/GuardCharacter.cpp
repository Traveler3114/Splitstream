// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/TimeObjects/GhostCharacterActor.h"
#include "GuardCharacter.h"
#include "Actors/NavNode.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogGuardCharacter, Log, All);

// Sets default values
AGuardCharacter::AGuardCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AGuardCharacter::BeginPlay()
{
	Super::BeginPlay();

	const bool bAuth = HasAuthority();
	// Spawn ghost on the server so it replicates, then mirror this character
	if (bAuth)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.Instigator = GetInstigator();
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		SpawnedGhost = GetWorld()->SpawnActor<AGhostCharacterActor>(
			AGhostCharacterActor::StaticClass(),
			GetActorLocation(),
			GetActorRotation(),
			Params);

		if (SpawnedGhost)
		{
			SpawnedGhost->CharacterToMirror = this;
		}
	}
}