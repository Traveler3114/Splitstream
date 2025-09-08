// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Computer.h"
#include "ArchiveComputer.generated.h"

UCLASS()
class ECHOESOFTIME_API AArchiveComputer : public AComputer
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AArchiveComputer();

    // Archive-specific properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archive")
    FString ComputerName = TEXT("Archive Terminal");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archive")
    int32 ArchiveIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archive")
    bool bIsMainArchiveTerminal = false;

    // Get a unique display name for this archive computer
    UFUNCTION(BlueprintCallable, Category = "Archive")
    FString GetDisplayName() const;

    // Set a procedural name for this archive computer
    UFUNCTION(BlueprintCallable, Category = "Archive")
    void SetProceduralName(const FString& BaseName, int32 Index);

protected:
    virtual void BeginPlay() override;
    virtual void Interact_Implementation(AActor* Interactor) override;

private:
    // Static array of archive terminal names for procedural generation
    static const TArray<FString> ArchiveTerminalNames;
};