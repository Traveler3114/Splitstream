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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadScanner")
    int32 Rows = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadScanner")
    int32 Columns = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Linked")
    AActor* LinkedActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadScanner")
    FString CorrectCode = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadScanner")
    EItemType RequiredKeycardType = EItemType::KeycardL2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keypad")
    int32 CodeLength = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keypad")
    float CodeLifetime = 60.f;

    // --- Replication ---
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(ReplicatedUsing = OnRep_EnteredCode)
    FString EnteredCode;

    UFUNCTION()
    void OnRep_EnteredCode();

    FTimerHandle CodeExpiryHandle;

    // --- API for CodeGenerator ---
    UFUNCTION(BlueprintCallable, Category = "KeypadScanner")
    void SetCodeWithExpiry(const FString& NewCode, float LifetimeSeconds);

    UFUNCTION(BlueprintCallable, Category = "KeypadScanner")
    void ClearCode();

    // --- Your original method ---
    UFUNCTION(BlueprintCallable, Category = "KeypadScanner")
    void SetCorrectCode(const FString& Code);

    // --- Interaction ---
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;
    virtual bool IsCorrectItem_Implementation(UItemBase* Item) const override;
    virtual bool RequiresItem_Implementation() const override { return true; }

    UFUNCTION()
    void AppendCodeSymbol(const FString& Symbol);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

protected:
    void SpawnKeypadButtons();

    bool bCodeCorrect = false;
    bool bUnlocked = false;

    void TryUnlock(AActor* Interactor);

    void SetEnteredCodeAndUpdateText(const FString& NewCode);
};