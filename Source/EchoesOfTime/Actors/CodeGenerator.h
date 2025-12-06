#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "Interfaces/IRequiresItem.h"
#include "TimelineEra.h"
#include "CodeGenerator.generated.h"

class AKeypadScanner;
class UTextRenderComponent;

USTRUCT(BlueprintType)
struct FKeypadCodeStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString Code;

    UPROPERTY(BlueprintReadOnly)
    float ExpiryTime;

    UPROPERTY(BlueprintReadOnly)
    AKeypadScanner* Keypad;

    FKeypadCodeStatus() : Code(TEXT("")), ExpiryTime(0.f), Keypad(nullptr) {}
    FKeypadCodeStatus(const FString& InCode, float InExpiry, AKeypadScanner* InKeypad)
        : Code(InCode), ExpiryTime(InExpiry), Keypad(InKeypad) {
    }
};

UCLASS()
class ECHOESOFTIME_API ACodeGenerator : public AActor, public IInteractable, public IRequiresItem
{
    GENERATED_BODY()

public:
    ACodeGenerator();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keypads")
    TArray<AKeypadScanner*> ManagedKeypads;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Desk")
    USceneComponent* DefaultSceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Desk")
    UStaticMeshComponent* CodeGenMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Display")
    UTextRenderComponent* CodesTextComp;

    // Status for each keypad, not replicated
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<FKeypadCodeStatus> StatusArray;

    // Replicated for Civilian
    UPROPERTY(Replicated, BlueprintReadOnly)
    class ACivilianCharacter* TargetCivilian = nullptr;

    // Replicated text for code/timer display
    UPROPERTY(ReplicatedUsing = OnRep_CodesDisplayText)
    FString CodesDisplayText;

    UFUNCTION()
    void OnRep_CodesDisplayText();

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;
    virtual bool IsCorrectItem_Implementation(UItemBase* Item) const override;

    void UpdateDisplayText();

private:
    // Timer-based update instead of Tick for better performance
    FTimerHandle CodeUpdateTimerHandle;
    void CheckExpiredCodes();
};