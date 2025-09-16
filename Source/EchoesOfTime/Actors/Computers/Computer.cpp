#include "Computer.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "HackingSystem/HackComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

AComputer::AComputer()
{
    PrimaryActorTick.bCanEverTick = false;

    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    ComputerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ComputerMesh"));
    ComputerMesh->SetupAttachment(DefaultSceneRoot);

    NameText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NameText"));
    NameText->SetupAttachment(DefaultSceneRoot);

    NameText->SetHorizontalAlignment(EHTA_Center);
    NameText->SetWorldSize(32.f);
    NameText->SetTextRenderColor(FColor::Cyan);
    NameText->SetRelativeLocation(FVector(0, 0, 100));
}

void AComputer::BeginPlay()
{
    Super::BeginPlay();

    if (NameText) NameText->SetText(FText::FromString(StaffName));

    HackComponent = FindComponentByClass<UHackComponent>();
    if (HackComponent)
    {
        HackComponent->OnHackComplete.AddDynamic(this, &AComputer::OnHackComplete);
    }
}

void AComputer::OnRep_StaffName()
{
    if (NameText) NameText->SetText(FText::FromString(StaffName));
}

void AComputer::SetupComputer(const FString& NewStaffName, const FString& NewStoredCode)
{
    StaffName = NewStaffName;
    StoredCode = NewStoredCode;
    if (NameText) NameText->SetText(FText::FromString(StaffName));
}

void AComputer::Interact_Implementation(AActor* Interactor)
{
    if (HackComponent)
    {
        HackComponent->Interact(Interactor);
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
    if (HasAuthority() && GEngine)
    {
        FString RevealMsg = FString::Printf(TEXT("Hacked! Keypad Code: %s"), *StoredCode);
        GEngine->AddOnScreenDebugMessage(-1, 6.0f, FColor::Green, RevealMsg);
    }
}

void AComputer::OnRep_StoredCode()
{
    // Optionally display code in UI/widgets
}

void AComputer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AComputer, StoredCode);
    DOREPLIFETIME(AComputer, StaffName);
}