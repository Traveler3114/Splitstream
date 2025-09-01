#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "Interfaces/IKeycardUnlockable.h"
#include "KeypadScanner.generated.h"

class AKeypadButton;

UCLASS()
class ECHOESOFTIME_API AKeypadScanner : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    AKeypadScanner();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

    // COMPONENTS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadScanner")
    USceneComponent* DefaultSceneRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadScanner")
    UStaticMeshComponent* KeypadScannerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadButton")
    class UTextRenderComponent* CodeTextRenderComp;

    // BUTTONS
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KeypadScanner")
    TArray<AKeypadButton*> KeypadButtons;

    UPROPERTY(EditDefaultsOnly, Category = "KeypadScanner")
    TSubclassOf<AKeypadButton> KeypadButtonClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadScanner")
    FVector ButtonGridOffset = FVector(-190.0f, -5.0f, 0.0f);

    // LINKED ACTOR (door, etc)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Linked")
    AActor* LinkedActor;

    // CODE DATA
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadScanner")
    FString CorrectCode = TEXT("1234");

    // IInteractable
    virtual void Interact_Implementation(AActor* Interactor) override;

    // BUTTON HANDLER
    UFUNCTION()
    void AppendCodeSymbol(const FString& Symbol);

protected:
    void SpawnKeypadButtons();

    // --- STATE ---
    FString EnteredCode;
    bool bCodeCorrect = false;
    bool bUnlocked = false;

    void TryUnlock(AActor* Interactor);
};