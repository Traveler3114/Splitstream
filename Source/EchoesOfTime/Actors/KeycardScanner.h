#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    ADoorBase* LinkedDoor;

    // IInteractable
    virtual void Interact_Implementation(AActor* Interactor) override;
};