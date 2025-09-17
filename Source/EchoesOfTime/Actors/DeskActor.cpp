#include "DeskActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Net/UnrealNetwork.h"
#include "Actors/PointActors/SearchableItemSpawnPoint.h"
#include "Actors/Computers/Computer.h"

ADeskActor::ADeskActor()
{
    PrimaryActorTick.bCanEverTick = false;

    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    DeskMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DeskMesh"));
    DeskMesh->SetupAttachment(DefaultSceneRoot);

    NameText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NameText"));
    NameText->SetupAttachment(DefaultSceneRoot);
    NameText->SetHorizontalAlignment(EHTA_Center);
    NameText->SetWorldSize(32.f);
    NameText->SetTextRenderColor(FColor::Cyan);
    NameText->SetRelativeLocation(FVector(0, 0, 100));
}

void ADeskActor::BeginPlay()
{
    Super::BeginPlay();
    OnRep_StaffName();
}

void ADeskActor::SetStaffName(const FString& NewName)
{
    StaffName = NewName;
    OnRep_StaffName();
}

void ADeskActor::OnRep_StaffName()
{
    if (NameText) NameText->SetText(FText::FromString(StaffName));
}

void ADeskActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ADeskActor, StaffName);
}