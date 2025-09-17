#include "NewspaperActor.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Net/UnrealNetwork.h"

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

    bReplicates = true;
}

void ANewspaperActor::BeginPlay()
{
    Super::BeginPlay();
    // Ensure text is up to date on client join
    if (!NewspaperDateString.IsEmpty())
    {
        if (DateText)
        {
            DateText->SetText(FText::FromString(NewspaperDateString));
        }
    }
}

void ANewspaperActor::SetDateText(const FString& DateStr)
{
    NewspaperDateString = DateStr;
    if (DateText)
    {
        DateText->SetText(FText::FromString(DateStr));
    }
    // DO NOT call OnRep_NewspaperDateString here!
}

void ANewspaperActor::OnRep_NewspaperDateString()
{
    if (DateText)
    {
        DateText->SetText(FText::FromString(NewspaperDateString));
    }
}

void ANewspaperActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ANewspaperActor, NewspaperDateString);
}