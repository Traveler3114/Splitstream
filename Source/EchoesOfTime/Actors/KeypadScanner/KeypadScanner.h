#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KeypadScanner.generated.h"

class AKeypadButton;

UCLASS()
class ECHOESOFTIME_API AKeypadScanner : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AKeypadScanner();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadScanner")
    USceneComponent* DefaultSceneRoot;

    // Static mesh child
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadScanner")
    UStaticMeshComponent* KeypadScannerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadButton")
    class UTextRenderComponent* CodeTextRenderComp;

    // Spawned keypad buttons
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KeypadScanner")
    TArray<AKeypadButton*> KeypadButtons;

    // Button class to spawn
    UPROPERTY(EditDefaultsOnly, Category = "KeypadScanner")
    TSubclassOf<AKeypadButton> KeypadButtonClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadScanner")
    FVector ButtonGridOffset = FVector(-190.0f, -5.0f, 0.0f);

    // Method to append code symbol, must be UFUNCTION() for delegate binding
    UFUNCTION()
    void AppendCodeSymbol(const FString& Symbol);

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    void SpawnKeypadButtons();

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;
};