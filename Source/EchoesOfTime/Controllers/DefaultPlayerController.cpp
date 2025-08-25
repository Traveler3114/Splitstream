#include "DefaultPlayerController.h"
#include "Characters/DefaultCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Widgets/HUD/CharacterHUD.h"

ADefaultPlayerController::ADefaultPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ADefaultPlayerController::BeginPlay()
{
    Super::BeginPlay();
    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;
    if (CharacterHUD)
    {
        if (CharacterHUD->CharacterOverlay == nullptr) CharacterHUD->AddCharacterOverlay();
    }
}

