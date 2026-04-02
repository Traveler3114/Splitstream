#include "NavNode.h"
#include "Components/SceneComponent.h"
#include "Components/ArrowComponent.h"

ANavNode::ANavNode()
{
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
    ArrowComp->SetupAttachment(Root);
}