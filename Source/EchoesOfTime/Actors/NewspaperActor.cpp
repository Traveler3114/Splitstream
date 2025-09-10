#include "NewspaperActor.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"

ANewspaperActor::ANewspaperActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create DefaultSceneRoot
    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    // Create NewspaperMesh and attach to root
    NewspaperMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NewspaperMesh"));
    NewspaperMesh->SetupAttachment(DefaultSceneRoot);

    // Create DateText and attach to mesh
    DateText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("DateText"));
    DateText->SetupAttachment(NewspaperMesh);
}

void ANewspaperActor::BeginPlay()
{
    Super::BeginPlay();
}

void ANewspaperActor::SetDateText(const FString& DateStr)
{
    if (DateText)
    {
        DateText->SetText(FText::FromString(DateStr));
    }
}