#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "Interfaces/IKeycardUnlockable.h"
#include "InventorySystem/ItemBase.h" // <-- Include for EItemType
#include "TimelineEra.h"
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Linked")
    AActor* LinkedActor;

    // **NEW:** Keycard type required to unlock
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keycard")
    EItemType RequiredKeycardType = EItemType::KeycardL1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    // IInteractable
    virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void SetHighlighted_Implementation(bool bHighlight) override;
};