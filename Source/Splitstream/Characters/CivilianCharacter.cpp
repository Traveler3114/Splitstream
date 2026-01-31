#include "CivilianCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Actors/SearchableActor.h"
#include "Perception/AISenseConfig_Sight.h"

ACivilianCharacter::ACivilianCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ACivilianCharacter::BeginPlay()
{
    Super::BeginPlay();
}