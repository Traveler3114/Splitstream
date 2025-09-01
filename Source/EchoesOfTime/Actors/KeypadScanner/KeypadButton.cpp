#include "KeypadButton.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"

AKeypadButton::AKeypadButton()
{
    PrimaryActorTick.bCanEverTick = true;

    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    KeypadButtonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KeypadButtonMesh"));
    KeypadButtonMesh->SetupAttachment(DefaultSceneRoot);

    NumberTextRenderComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NumberTextRenderComp"));
    NumberTextRenderComp->SetupAttachment(KeypadButtonMesh);

    NumberTextRenderComp->SetText(FText::FromString(TEXT("n")));
}

void AKeypadButton::BeginPlay()
{
    Super::BeginPlay();
}

void AKeypadButton::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AKeypadButton::Interact_Implementation(AActor* Interactor)
{
    OnButtonPressed.Broadcast(ButtonSymbol);
}