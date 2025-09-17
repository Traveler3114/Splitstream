#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimelineEra.h"
#include "CivilianCharacter.generated.h"

UCLASS()
class ECHOESOFTIME_API ACivilianCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ACivilianCharacter();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian")
    FString CivilianName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian")
    UTexture2D* PortraitTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian")
    class ADeskActor* AssignedDesk = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;
};