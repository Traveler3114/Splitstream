#pragma once

#include "CoreMinimal.h"
#include "AICharacter.h"
#include "Interfaces/IDetectable.h"
#include "Interfaces/IInteractable.h"
#include "AbilitySystemInterface.h"
#include "DataAssets/ItemBase.h"
#include "TimelineEra.h"
#include "CivilianCharacter.generated.h"

class ADeskActor;

UCLASS()
class SPLITSTREAM_API ACivilianCharacter : public AAICharacter
{
    GENERATED_BODY()

public:

    ACivilianCharacter();
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian")
    FString CivilianName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian")
    UTexture2D* PortraitTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian")
    ADeskActor* AssignedDesk = nullptr;
};