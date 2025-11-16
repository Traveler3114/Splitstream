#include "AlarmButton.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameStates/DefaultGameState.h"

AAlarmButton::AAlarmButton()
{
    RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
    RootComponent = RootComp;

    AlarmButtonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AlarmButtonMesh"));
    AlarmButtonMesh->SetupAttachment(RootComponent);
}

void AAlarmButton::Interact_Implementation(AActor* Interactor)
{
    if (ADefaultGameState* GS = Cast<ADefaultGameState>(GetWorld()->GetGameState()))
    {
        GS->StartAlarm();
    }
}

void AAlarmButton::SetHighlighted_Implementation(bool bHighlight)
{
    if (AlarmButtonMesh)
    {
        AlarmButtonMesh->SetRenderCustomDepth(bHighlight);
        AlarmButtonMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}