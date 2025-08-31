#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "Interfaces/IKeycardUnlockable.h"
#include "KeycardScanner.generated.h"

UCLASS()
class ECHOESOFTIME_API AKeycardScanner : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    AKeycardScanner();
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    USceneComponent* DefaultSceneRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* Mesh;

    // The actor to unlock with a keycard
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Linked")
    AActor* LinkedActor;

    // IInteractable
    virtual void Interact_Implementation(AActor* Interactor) override;
};