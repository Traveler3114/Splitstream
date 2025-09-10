#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "TimelineEra.h"
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Computer")
    USceneComponent* DefaultSceneRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Computer")
    UStaticMeshComponent* ComputerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Computer")
    UTextRenderComponent* NameText;

    UPROPERTY(ReplicatedUsing = OnRep_StoredCode, VisibleAnywhere, BlueprintReadOnly, Category = "Hack")
    FString StoredCode;

    UPROPERTY(ReplicatedUsing = OnRep_StaffName, VisibleAnywhere, BlueprintReadOnly, Category = "Staff")
    FString StaffName;

    UFUNCTION(BlueprintCallable, Category = "Setup")
    void SetupComputer(const FString& NewStaffName, const FString& NewStoredCode);

    UFUNCTION()
    void OnRep_StoredCode();

    UFUNCTION()
    void OnHackComplete();

    UFUNCTION()
    void OnRep_StaffName();

protected:
    virtual void BeginPlay() override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;

    UPROPERTY()
    UHackComponent* HackComponent = nullptr;
};