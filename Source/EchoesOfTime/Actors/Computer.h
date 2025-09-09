#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "Computer.generated.h"

class UHackComponent;
class UTextRenderComponent;

UCLASS()
class ECHOESOFTIME_API AComputer : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    AComputer();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Computer")
    class USceneComponent* DefaultSceneRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Computer")
    class UStaticMeshComponent* ComputerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Computer")
    class UTextRenderComponent* NameText; // <--- ADD THIS

    UHackComponent* HackComponent = nullptr;

    UPROPERTY(ReplicatedUsing = OnRep_StoredCode, VisibleAnywhere, BlueprintReadOnly, Category = "Hack")
    FString StoredCode;

    UFUNCTION()
    void OnRep_StoredCode();

    UFUNCTION(BlueprintCallable, Category = "Hack")
    void SetStoredCode(const FString& Code) { StoredCode = Code; }

    UFUNCTION(BlueprintCallable, Category = "Hack")
    FString RevealStoredCode() const { return StoredCode; }

    UFUNCTION()
    void OnHackComplete();

    // --- NEW CODE: Unique Staff Name ---
    UPROPERTY(ReplicatedUsing = OnRep_StaffName, VisibleAnywhere, BlueprintReadOnly, Category = "Staff")
    FString StaffName;

    UFUNCTION()
    void OnRep_StaffName();

    UFUNCTION(BlueprintCallable, Category = "Staff")
    void SetStaffName(const FString& Name);

protected:
    virtual void BeginPlay() override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;
};