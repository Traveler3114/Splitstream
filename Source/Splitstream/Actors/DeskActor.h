#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimelineEra.h"
#include "DeskActor.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;
class AComputer;
class ASearchableItemSpawnPoint;

UCLASS()
class ECHOESOFTIME_API ADeskActor : public AActor
{
    GENERATED_BODY()

public:
    ADeskActor();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(ReplicatedUsing = OnRep_StaffName, VisibleAnywhere, BlueprintReadOnly, Category = "Staff")
    FString StaffName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desk")
    AComputer* DeskComputer = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desk")
    TArray<ASearchableItemSpawnPoint*> ItemSpawnPoints;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Desk")
    USceneComponent* DefaultSceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Desk")
    UStaticMeshComponent* DeskMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Desk")
    UTextRenderComponent* NameText;

    UFUNCTION(BlueprintCallable, Category = "Desk")
    void SetStaffName(const FString& NewName);

    UFUNCTION()
    void OnRep_StaffName();

protected:
    virtual void BeginPlay() override;
};