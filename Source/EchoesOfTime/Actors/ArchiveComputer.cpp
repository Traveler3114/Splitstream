// Fill out your copyright notice in the Description page of Project Settings.

#include "ArchiveComputer.h"
#include "ProceduralLevelManager.h"
#include "../Widgets/HUD/ArchiveCalendarWidget.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

AArchiveComputer::AArchiveComputer()
{
    // Set default values specific to archive computers
    PrimaryActorTick.bCanEverTick = true;
}

void AArchiveComputer::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize archive data
    InitializeArchiveData();
}

void AArchiveComputer::Interact_Implementation(AActor* Interactor)
{
    // Call parent implementation first (handles hacking)
    Super::Interact_Implementation(Interactor);
    
    // If archive is unlocked, open the interface
    if (bArchiveUnlocked)
    {
        OpenArchiveInterface(Interactor);
    }
    else
    {
        // Show message that archive needs to be hacked first
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                TEXT("Archive computer is locked. Hack to gain access."));
        }
    }
}

void AArchiveComputer::OnHackComplete()
{
    // Call parent implementation
    Super::OnHackComplete();
    
    // Unlock archive access
    bArchiveUnlocked = true;
    CurrentAccessLevel = EArchiveAccessLevel::Level1; // Basic access after hack
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("Archive Access Granted! Historical records now available."));
    }
    
    // If someone is currently interacting, open the interface
    if (CurrentUser)
    {
        OpenArchiveInterface(CurrentUser);
    }
}

void AArchiveComputer::OpenArchiveInterface(AActor* User)
{
    if (!bArchiveUnlocked || !User)
    {
        return;
    }
    
    CurrentUser = User;
    
    // Create widget if it doesn't exist
    if (!ArchiveWidget)
    {
        CreateArchiveWidget();
    }
    
    if (ArchiveWidget)
    {
        // Initialize widget with this computer
        ArchiveWidget->InitializeWidget(this);
        
        // Add to viewport
        ArchiveWidget->AddToViewport();
        
        // Enable mouse cursor for interaction
        if (APlayerController* PC = Cast<APlayerController>(User))
        {
            PC->bShowMouseCursor = true;
            PC->SetInputMode(FInputModeUIOnly());
        }
        
        UE_LOG(LogTemp, Log, TEXT("Archive interface opened for user"));
    }
}

void AArchiveComputer::CloseArchiveInterface()
{
    if (ArchiveWidget)
    {
        ArchiveWidget->RemoveFromParent();
        
        // Restore input mode
        if (CurrentUser)
        {
            if (APlayerController* PC = Cast<APlayerController>(CurrentUser))
            {
                PC->bShowMouseCursor = false;
                PC->SetInputMode(FInputModeGameOnly());
            }
        }
        
        CurrentUser = nullptr;
        UE_LOG(LogTemp, Log, TEXT("Archive interface closed"));
    }
}

TArray<FString> AArchiveComputer::GetArchiveEntries() const
{
    TArray<FString> Entries;
    ArchiveEntries.GetKeys(Entries);
    return Entries;
}

FString AArchiveComputer::GetArchiveEntryByDate(const FString& Date) const
{
    if (const FString* Entry = ArchiveEntries.Find(Date))
    {
        return *Entry;
    }
    return FString();
}

void AArchiveComputer::InitializeArchiveData()
{
    // Clear existing entries
    ArchiveEntries.Empty();
    
    // Add some default archive entries with dates and content
    ArchiveEntries.Add(TEXT("2157.03.15"), 
        TEXT("Temporal displacement detected in Sector 7. Research team deployed."));
    
    ArchiveEntries.Add(TEXT("2157.03.20"), 
        TEXT("First contact with temporal anomaly confirmed. Establishing containment protocols."));
    
    ArchiveEntries.Add(TEXT("2157.04.02"), 
        TEXT("Anomaly shows signs of intelligence. Communication attempts underway."));
    
    ArchiveEntries.Add(TEXT("2157.04.15"), 
        TEXT("Breakthrough: Temporal entity responds to quantum frequencies. Archive classification: ECHO-1."));
    
    ArchiveEntries.Add(TEXT("2157.05.01"), 
        TEXT("ECHO-1 demonstrates ability to manipulate local timestreams. Facility lockdown initiated."));
    
    ArchiveEntries.Add(TEXT("2157.05.10"), 
        TEXT("Final log: ECHO-1 has merged with facility systems. Evacuation in progress. Time distortions increasing."));
    
    // Get procedural manager for additional entries
    if (AProceduralLevelManager* Manager = AProceduralLevelManager::GetInstance(GetWorld()))
    {
        // Add procedurally generated clue entries
        TArray<FString> GeneratedCodes = Manager->GetAllCodes();
        for (const FString& Code : GeneratedCodes)
        {
            FClueData ClueData = Manager->GetRandomClueForCode(Code);
            if (!ClueData.ClueText.IsEmpty())
            {
                FString DateKey = FString::Printf(TEXT("2157.06.%02d"), FMath::RandRange(1, 30));
                ArchiveEntries.Add(DateKey, ClueData.ClueText);
            }
        }
    }
}

void AArchiveComputer::CreateArchiveWidget()
{
    if (ArchiveWidgetClass)
    {
        // Create widget from class
        if (UWorld* World = GetWorld())
        {
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                ArchiveWidget = CreateWidget<UArchiveCalendarWidget>(PC, ArchiveWidgetClass);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchiveWidgetClass not set for ArchiveComputer"));
    }
}