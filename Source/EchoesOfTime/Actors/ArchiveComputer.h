// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Computer.h"
#include "ArchiveComputer.generated.h"

// Forward declarations
class UArchiveCalendarWidget;

UENUM(BlueprintType)
enum class EArchiveAccessLevel : uint8
{
    None = 0,
    Level1 = 1,
    Level2 = 2,
    Level3 = 3,
    Full = 4
};

UCLASS()
class ECHOESOFTIME_API AArchiveComputer : public AComputer
{
    GENERATED_BODY()

public:
    AArchiveComputer();

    virtual void BeginPlay() override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void OnHackComplete() override;

    // Archive-specific functionality
    UFUNCTION(BlueprintCallable, Category = "Archive")
    void OpenArchiveInterface(AActor* User);

    UFUNCTION(BlueprintCallable, Category = "Archive")
    void CloseArchiveInterface();

    UFUNCTION(BlueprintCallable, Category = "Archive")
    bool IsArchiveAccessible() const { return bArchiveUnlocked; }

    UFUNCTION(BlueprintCallable, Category = "Archive")
    EArchiveAccessLevel GetAccessLevel() const { return CurrentAccessLevel; }

    UFUNCTION(BlueprintCallable, Category = "Archive")
    void SetAccessLevel(EArchiveAccessLevel NewLevel) { CurrentAccessLevel = NewLevel; }

    // Get archive entries for the calendar widget
    UFUNCTION(BlueprintCallable, Category = "Archive")
    TArray<FString> GetArchiveEntries() const;

    // Get specific archive entry by date
    UFUNCTION(BlueprintCallable, Category = "Archive")
    FString GetArchiveEntryByDate(const FString& Date) const;

protected:
    // Archive data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archive")
    TMap<FString, FString> ArchiveEntries;

    // Access control
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Archive")
    bool bArchiveUnlocked = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Archive")
    EArchiveAccessLevel CurrentAccessLevel = EArchiveAccessLevel::None;

    // Widget references
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Archive")
    TSubclassOf<UArchiveCalendarWidget> ArchiveWidgetClass;

    UPROPERTY()
    UArchiveCalendarWidget* ArchiveWidget = nullptr;

    // Current user interacting with the archive
    UPROPERTY()
    AActor* CurrentUser = nullptr;

    // Initialize default archive entries
    void InitializeArchiveData();

    // Create and show archive widget
    void CreateArchiveWidget();
};