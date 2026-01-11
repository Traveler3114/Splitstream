#include "Computer.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
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

    CodeText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("CodeText"));
    CodeText->SetupAttachment(DefaultSceneRoot);
    CodeText->SetHorizontalAlignment(EHTA_Center);
    CodeText->SetTextRenderColor(FColor::Green); // Or another color
    CodeText->SetText(FText::FromString(TEXT("")));
    CodeText->SetHiddenInGame(true); // Hide by default if you wish
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

    if (CodeText)
    {
        CodeText->SetHiddenInGame(true); // Always hide at setup
    }
}

void AComputer::Interact_Implementation(AActor* Interactor)
{
    if (HackComponent)
    {
        HackComponent->Interact(Interactor);
    }
}

bool AComputer::IsProgressiveInteract_Implementation()
{
    return true;
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
    OnRep_StoredCode();
}

void AComputer::OnRep_StoredCode()
{
    if (!HackComponent) return;

    // Only show code if the computer is hacked
    if (HackComponent->bHacked)
    {
        if (CodeText)
        {
            if (StoredCode.IsEmpty())
            {
                CodeText->SetText(FText::FromString(TEXT("Wrong")));
            }
            else
            {
                CodeText->SetText(FText::FromString(StoredCode));
            }
            CodeText->SetHiddenInGame(false);
        }
    }
    else
    {
        // Not hacked: hide the text
        if (CodeText) CodeText->SetHiddenInGame(true);
    }
}

void AComputer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AComputer, StoredCode);
}