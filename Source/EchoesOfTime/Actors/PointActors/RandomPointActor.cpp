#include "RandomPointActor.h"
#include "Components/SceneComponent.h"
#include "Components/ArrowComponent.h"

ARandomPointActor::ARandomPointActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Makes it visible in the editor
    USceneComponent* SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneComp"));
    RootComponent = SceneComp;

    ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
    ArrowComp->SetupAttachment(RootComponent);
}