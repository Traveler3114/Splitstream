#include "CivilianCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

ACivilianCharacter::ACivilianCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ACivilianCharacter::BeginPlay()
{
    Super::BeginPlay();
}