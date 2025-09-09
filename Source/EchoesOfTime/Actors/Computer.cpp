#include "Computer.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "HackingSystem/HackComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

// Sets default values
AComputer::AComputer()
{
    PrimaryActorTick.bCanEverTick = false;

    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    ComputerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ComputerMesh"));
    ComputerMesh->SetupAttachment(DefaultSceneRoot);

    NameText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NameText"));
    NameText->SetupAttachment(DefaultSceneRoot);

    // Set default properties for the name label
    NameText->SetHorizontalAlignment(EHTA_Center);
    NameText->SetWorldSize(32.f);
    NameText->SetTextRenderColor(FColor::Cyan);
    NameText->SetRelativeLocation(FVector(0, 0, 100)); // Offset above computer, tweak as needed
}

void AComputer::BeginPlay()
{
    Super::BeginPlay();

    // Set initial text for staff name
    if (NameText) NameText->SetText(FText::FromString(StaffName));

    HackComponent = FindComponentByClass<UHackComponent>();
    if (HackComponent)
    {
        HackComponent->OnHackComplete.AddDynamic(this, &AComputer::OnHackComplete);
    }
}

void AComputer::OnRep_StaffName()
{
    // Update the text in game when StaffName changes (networked)
    if (NameText) NameText->SetText(FText::FromString(StaffName));
}

void AComputer::SetStaffName(const FString& Name)
{
    StaffName = Name;
    if (NameText) NameText->SetText(FText::FromString(Name));
}

void AComputer::Interact_Implementation(AActor* Interactor)
{
    // You can use HackComp here to trigger hacking if present
    // if (HackComp) { ... }
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
    // Display the code on screen for the player
    if (HasAuthority())
    {
        if (GEngine)
        {
            FString RevealMsg = FString::Printf(TEXT("Hacked! Keypad Code: %s"), *StoredCode);
            GEngine->AddOnScreenDebugMessage(-1, 6.0f, FColor::Green, RevealMsg);
        }
    }
}

void AComputer::OnRep_StoredCode()
{
    // Optionally show the code to clients, e.g. if you want to display it in UI/widgets
}

void AComputer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AComputer, StoredCode);
    DOREPLIFETIME(AComputer, StaffName);
}