#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimelineEra.h"
#include "NewspaperActor.generated.h"

UCLASS()
class ECHOESOFTIME_API ANewspaperActor : public AActor
{
    GENERATED_BODY()

public:
    ANewspaperActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UTextRenderComponent* DateText;

    UFUNCTION(BlueprintCallable)
    void SetDateText(const FString& DateStr);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

protected:
    virtual void BeginPlay() override;
};