// Fill out your copyright notice in the Description page of Project Settings.

#include "ArchiveComputer.h"
#include "Engine/Engine.h"

// Static array of archive terminal names
const TArray<FString> AArchiveComputer::ArchiveTerminalNames = {
    TEXT("Data Node Alpha"),
    TEXT("Storage Terminal Beta"),
    TEXT("Archive Access Gamma"),
    TEXT("Memory Bank Delta"),
    TEXT("Information Hub Epsilon"),
    TEXT("Database Terminal Zeta"),
    TEXT("Archive Console Eta"),
    TEXT("Data Repository Theta"),
    TEXT("Storage Access Iota"),
    TEXT("Archive Terminal Kappa")
};

AArchiveComputer::AArchiveComputer()
{
    // Set default values
    ComputerName = TEXT("Archive Terminal");
    ArchiveIndex = 0;
    bIsMainArchiveTerminal = false;
}

void AArchiveComputer::BeginPlay()
{
    Super::BeginPlay();
    
    // If no custom name was set, generate a procedural one
    if (ComputerName == TEXT("Archive Terminal"))
    {
        SetProceduralName(TEXT("Archive Terminal"), ArchiveIndex);
    }
}

void AArchiveComputer::Interact_Implementation(AActor* Interactor)
{
    // Call parent interaction logic first
    Super::Interact_Implementation(Interactor);
    
    // Archive-specific interaction feedback
    if (GEngine && HasAuthority())
    {
        FString InteractionMsg = FString::Printf(TEXT("Accessing %s..."), *GetDisplayName());
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, InteractionMsg);
    }
}

FString AArchiveComputer::GetDisplayName() const
{
    return ComputerName;
}

void AArchiveComputer::SetProceduralName(const FString& BaseName, int32 Index)
{
    ArchiveIndex = Index;
    
    // Use predefined names if available, otherwise generate numbered names
    if (ArchiveTerminalNames.IsValidIndex(Index))
    {
        ComputerName = ArchiveTerminalNames[Index];
    }
    else
    {
        ComputerName = FString::Printf(TEXT("%s %d"), *BaseName, Index + 1);
    }
}