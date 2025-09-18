#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "Interfaces/IKeycardUnlockable.h"
#include "DataAssets/ItemBase.h"
#include "TimelineEra.h"
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadScanner")
    USceneComponent* DefaultSceneRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadScanner")
    UStaticMeshComponent* KeypadScannerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadButton")
    class UTextRenderComponent* CodeTextRenderComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KeypadScanner")
    TArray<AKeypadButton*> KeypadButtons;

    UPROPERTY(EditDefaultsOnly, Category = "KeypadScanner")
    TSubclassOf<AKeypadButton> KeypadButtonClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadScanner")
    FVector ButtonGridOffset = FVector(-190.0f, -5.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Linked")
    AActor* LinkedActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadScanner")
    FString CorrectCode = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadScanner")
    EItemType RequiredKeycardType = EItemType::KeycardL2;

    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;

    UFUNCTION()
    void AppendCodeSymbol(const FString& Symbol);

    // --- NEW CODE: External code setter ---
    UFUNCTION(BlueprintCallable, Category = "KeypadScanner")
    void SetCorrectCode(const FString& Code) { CorrectCode = Code; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    // --- REPLICATION ---
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Replicated code string
    UPROPERTY(ReplicatedUsing = OnRep_EnteredCode)
    FString EnteredCode;

    UFUNCTION()
    void OnRep_EnteredCode();

protected:
    void SpawnKeypadButtons();

    bool bCodeCorrect = false;
    bool bUnlocked = false;

    void TryUnlock(AActor* Interactor);

    // Helper to set code and update text everywhere
    void SetEnteredCodeAndUpdateText(const FString& NewCode);
};