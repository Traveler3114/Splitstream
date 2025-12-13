// SecurityDocumentActor.cpp

#include "SecurityDocumentActor.h"
#include "Components/ArrowComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SceneComponent.h"
#include "ProceduralLevelGenerator.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

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

    UE_LOG(LogTemp, Warning, TEXT("SecurityDocumentActor::BeginPlay on %s"),
        HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"));

    // On the server, after the document has spawned, ask the generator
    // to update all documents (including this one).
    if (HasAuthority())
    {
        UWorld* World = GetWorld();
        if (World)
        {
            AProceduralLevelGenerator* Gen =
                Cast<AProceduralLevelGenerator>(UGameplayStatics::GetActorOfClass(
                    World, AProceduralLevelGenerator::StaticClass()));

            if (Gen)
            {
                Gen->OnRep_PastWireDeviceSequence();
            }
        }
    }

    // Clients will be updated from OnRep_PastWireDeviceSequence
}