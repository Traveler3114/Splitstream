#include "CivilianSpawnPoint.h"
#include "Components/BillboardComponent.h"

ACivilianSpawnPoint::ACivilianSpawnPoint()
{
    PrimaryActorTick.bCanEverTick = false;
    // Visible marker in the editor
    UBillboardComponent* Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
    RootComponent = Billboard;
}