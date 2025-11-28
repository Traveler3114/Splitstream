#include "RandomPointActor.h"
#include "Components/SceneComponent.h"

ARandomPointActor::ARandomPointActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Makes it visible in the editor
    USceneComponent* SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneComp"));
    RootComponent = SceneComp;
}