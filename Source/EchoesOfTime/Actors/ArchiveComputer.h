#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "ArchiveComputer.generated.h"

class UCalendarWidget;
class AProceduralLevelGenerator;

UCLASS()
class ECHOESOFTIME_API AArchiveComputer : public AActor, public IInteractable
{
    GENERATED_BODY()
public:
    AArchiveComputer();
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Computer")
    class USceneComponent* DefaultSceneRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Computer")
    class UStaticMeshComponent* ComputerMesh;

    virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void SetHighlighted_Implementation(bool bHighlight) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archive")
    TSubclassOf<UCalendarWidget> CalendarWidgetClass;

    UPROPERTY(BlueprintReadWrite)
    UCalendarWidget* CalendarWidgetInstance;

    UPROPERTY(BlueprintReadOnly)
    AProceduralLevelGenerator* GeneratorRef;
};