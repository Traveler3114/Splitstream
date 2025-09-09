#include "RandomPointActor.h"
#include "Components/BillboardComponent.h"

ARandomPointActor::ARandomPointActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Makes it visible in the editor
    UBillboardComponent* Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
    RootComponent = Billboard;
}