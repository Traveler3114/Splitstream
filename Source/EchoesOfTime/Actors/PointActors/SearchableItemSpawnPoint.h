#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimelineEra.h"
#include "SearchableItemSpawnPoint.generated.h"

UENUM(BlueprintType)
enum class EItemSpawnArea : uint8
{
    ManagerOffice    UMETA(DisplayName = "Manager Office"),
    CivilianDesk     UMETA(DisplayName = "Civilian Desk"),
    BreakRoom        UMETA(DisplayName = "Break Room"),
    Storage          UMETA(DisplayName = "Storage"),
    Hallway          UMETA(DisplayName = "Hallway"),
    // Add more areas as needed
};

UCLASS()
class ECHOESOFTIME_API ASearchableItemSpawnPoint : public AActor
{
    GENERATED_BODY()
public:
    ASearchableItemSpawnPoint();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnPoint")
    EItemSpawnArea SpawnArea=EItemSpawnArea::CivilianDesk;

protected:
    virtual void BeginPlay() override;
};