#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "TimelineEra.h"
#include "AlarmButton.generated.h"

UCLASS()
class SPLITSTREAM_API AAlarmButton : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AAlarmButton();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;
    /** Scene Root Component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootComp;

    /** Static Mesh Component for the Alarm Button */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* AlarmButtonMesh;

    virtual void Interact_Implementation(AActor* Interactor) override;

    virtual void SetHighlighted_Implementation(bool bHighlight) override;
};