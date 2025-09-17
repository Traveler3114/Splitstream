#include "CivilianCharacter.h"
#include "Actors/DeskActor.h"

ACivilianCharacter::ACivilianCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
    // No replication needed for CivilianName or PortraitTexture in this design.
}