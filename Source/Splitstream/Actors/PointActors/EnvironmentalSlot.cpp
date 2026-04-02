// AEnvironmentalSlot.cpp
#include "EnvironmentalSlot.h"

#if WITH_EDITORONLY_DATA
#include "Components/ArrowComponent.h"
#endif

AEnvironmentalSlot::AEnvironmentalSlot()
{
    PrimaryActorTick.bCanEverTick = false;

#if WITH_EDITORONLY_DATA
    ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("FacingDirection"));
    ArrowComponent->SetupAttachment(RootComponent);
    ArrowComponent->ArrowColor = FColor(0, 200, 255);
    ArrowComponent->bIsScreenSizeScaled = true;
#endif
}

void AEnvironmentalSlot::Occupy(AActor* NPC)
{
    OccupyingNPC = NPC;
}

void AEnvironmentalSlot::Release()
{
    OccupyingNPC = nullptr;
}
