// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "Computer.generated.h"

// Forward declaration
class UHackComponent;

UCLASS()
class ECHOESOFTIME_API AComputer : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AComputer();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Computer")
    class USceneComponent* DefaultSceneRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Computer")
    class UStaticMeshComponent* ComputerMesh;

    UHackComponent* HackComponent = nullptr;

    UPROPERTY(ReplicatedUsing = OnRep_StoredCode, VisibleAnywhere, BlueprintReadOnly, Category = "Hack")
    FString StoredCode;

    UFUNCTION()
    void OnRep_StoredCode();

    // Setter
    UFUNCTION(BlueprintCallable, Category = "Hack")
    void SetStoredCode(const FString& Code) { StoredCode = Code; }

    // You can add a function to reveal the code when hacked
    UFUNCTION(BlueprintCallable, Category = "Hack")
    FString RevealStoredCode() const { return StoredCode; }

    UFUNCTION()
    void OnHackComplete();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;

};