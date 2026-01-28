// FingerprintScanner.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "DataAssets/ItemBase.h"
#include "Characters/CivilianCharacter.h"
#include "TimelineEra.h"
#include "FingerprintScanner.generated.h"

UCLASS()
class SPLITSTREAM_API AFingerprintScanner : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    AFingerprintScanner();
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    USceneComponent* DefaultSceneRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* Mesh;

    // Optional: link to an actor to unlock when access granted
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Linked")
    AActor* LinkedActor;

    // What type of civilian's fingerprint is required?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fingerprint")
    ECivilianType RequiredCivilianType = ECivilianType::ExecutiveManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    // IInteractable overrides
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;
    virtual bool IsCorrectItem_Implementation(UItemBase* Item) const override;
    virtual bool RequiresItem_Implementation() const override { return true; }
};