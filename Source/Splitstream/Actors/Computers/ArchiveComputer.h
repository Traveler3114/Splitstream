#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "TimelineEra.h"
#include "ArchiveComputer.generated.h"



class AProceduralLevelGenerator;
class AComputer;
class ACivilianCharacter;
class ADeskActor;

UCLASS()
class SPLITSTREAM_API AArchiveComputer : public AActor, public IInteractable
{
    GENERATED_BODY()
public:
    AArchiveComputer();
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Computer")
    USceneComponent* DefaultSceneRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Computer")
    UStaticMeshComponent* ComputerMesh;

    // Store computers with non-empty code
    UPROPERTY(BlueprintReadOnly)
    TArray<AComputer*> CodeComputers;

    // Map for fast lookup: Computer -> Civilian
    UPROPERTY()
    TMap<AComputer*, ACivilianCharacter*> ComputerToCivilianMap;

    // Map for fast lookup: Computer -> Desk
    UPROPERTY()
    TMap<AComputer*, ADeskActor*> ComputerToDeskMap;

    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;

    UPROPERTY(BlueprintReadOnly)
    AProceduralLevelGenerator* GeneratorRef;
};