#include "Computer.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "ActorComponents/HackComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

AComputer::AComputer()
{
    PrimaryActorTick.bCanEverTick = false;

    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    ComputerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ComputerMesh"));
    ComputerMesh->SetupAttachment(DefaultSceneRoot);
}

void AComputer::BeginPlay()
{
    Super::BeginPlay();

    HackComponent = FindComponentByClass<UHackComponent>();
    if (HackComponent)
    {
        HackComponent->OnHackComplete.AddDynamic(this, &AComputer::OnHackComplete);
    }
}



void AComputer::SetupComputer(const FString& NewStaffName, const FString& NewStoredCode)
{

    StoredCode = NewStoredCode;
}

void AComputer::Interact_Implementation(AActor* Interactor)
{
    if (HackComponent)
    {
        HackComponent->Interact(Interactor);
    }
}

void AComputer::CancelInteract_Implementation(AActor* Interactor)
{
    if (HackComponent)
    {
        HackComponent->CancelInteract(Interactor);
    }
}

void AComputer::SetHighlighted_Implementation(bool bHighlight)
{
    if (ComputerMesh && HackComponent)
    {
        if (HackComponent->bHacked)
        {
            ComputerMesh->SetRenderCustomDepth(false);
            ComputerMesh->CustomDepthStencilValue = 0;
        }
        else
        {
            ComputerMesh->SetRenderCustomDepth(bHighlight);
            ComputerMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
        }
    }
}

void AComputer::OnHackComplete()
{
}
void AComputer::OnRep_StoredCode()
{
    // Optionally display code in UI/widgets
}

void AComputer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AComputer, StoredCode);
}