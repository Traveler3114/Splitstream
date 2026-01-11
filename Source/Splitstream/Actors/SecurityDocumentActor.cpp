// SecurityDocumentActor.cpp

#include "SecurityDocumentActor.h"
#include "Components/ArrowComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SceneComponent.h"

ASecurityDocumentActor::ASecurityDocumentActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
    ArrowComp->SetupAttachment(RootComponent);
    ArrowComp->ArrowSize = 1.0f;
    ArrowComp->bIsScreenSizeScaled = true;

    WidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComp"));
    WidgetComp->SetupAttachment(RootComponent);
    WidgetComp->SetDrawSize(FVector2D(50.f, 75.f));
    WidgetComp->SetWidgetSpace(EWidgetSpace::World);

    bReplicates = true;
}

void ASecurityDocumentActor::BeginPlay()
{
    Super::BeginPlay();
}