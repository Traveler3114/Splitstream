#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimelineEra.h"
#include "NewspaperActor.generated.h"

UCLASS()
class SPLITSTREAM_API ANewspaperActor : public AActor
{
    GENERATED_BODY()

public:
    ANewspaperActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UTextRenderComponent* DateText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UStaticMeshComponent* NewspaperMesh;

    UPROPERTY(VisibleAnywhere)
    class USceneComponent* DefaultSceneRoot;

    // Replicated date string; when changed, will update text on all clients
    UPROPERTY(ReplicatedUsing = OnRep_NewspaperDateString)
    FString NewspaperDateString;

    UFUNCTION()
    void OnRep_NewspaperDateString();

    UFUNCTION(BlueprintCallable)
    void SetDateText(const FString& DateStr);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

protected:
    virtual void BeginPlay() override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};