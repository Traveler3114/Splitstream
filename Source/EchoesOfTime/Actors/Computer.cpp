// Fill out your copyright notice in the Description page of Project Settings.

#include "Computer.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "HackingSystem/HackComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AComputer::AComputer()
{
    PrimaryActorTick.bCanEverTick = false;

    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    ComputerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ComputerMesh"));
    ComputerMesh->SetupAttachment(DefaultSceneRoot);
}

// Called when the game starts or when spawned
void AComputer::BeginPlay()
{
    Super::BeginPlay();

    // Find and cache optional hack component
    HackComponent = FindComponentByClass<UHackComponent>();
    if (HackComponent)
    {
        HackComponent->OnHackComplete.AddDynamic(this, &AComputer::OnHackComplete);
    }
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
        // If hacked, always disable highlighting
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
}