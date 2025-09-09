#include "NewspaperActor.h"
#include "Components/TextRenderComponent.h"

ANewspaperActor::ANewspaperActor()
{
    PrimaryActorTick.bCanEverTick = false;

    DateText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("DateText"));
    RootComponent = DateText;
    DateText->SetWorldSize(40.f);
    DateText->SetHorizontalAlignment(EHTA_Center);
    DateText->SetTextRenderColor(FColor::White);
    DateText->SetRelativeLocation(FVector(0,0,0)); // adjust as needed
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