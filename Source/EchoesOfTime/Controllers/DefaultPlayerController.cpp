#include "DefaultPlayerController.h"
#include "LockPickingSystem/LockPickComponent.h"
#include "Widgets/HUD/CharacterHUD.h"

ADefaultPlayerController::ADefaultPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
    CharacterHUD = nullptr;
}

void ADefaultPlayerController::BeginPlay()
{
    Super::BeginPlay();
    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;
    if (CharacterHUD)
    {
        if (CharacterHUD->CharacterOverlay == nullptr)
            CharacterHUD->AddCharacterOverlay();
    }
}

void ADefaultPlayerController::ServerLockPickConfirm_Implementation(AActor* DoorActor, float Angle)
{
    if (!DoorActor) return;
    ULockPickComponent* LockComp = DoorActor->FindComponentByClass<ULockPickComponent>();
    if (LockComp)
    {
        if (LockComp->TrySetCurrentPin(Angle))
        {
            if (LockComp->AdvancePin())
            {
                LockComp->EndLockPicking();
            }
        }
    }
}