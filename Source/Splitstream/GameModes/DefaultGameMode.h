#pragma once

#include "CoreMinimal.h"
#include "BaseGameMode.h"
#include "TimelineEra.h"
#include "DefaultGameMode.generated.h"

UCLASS()
class SPLITSTREAM_API ADefaultGameMode : public ABaseGameMode
{
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

    UFUNCTION(BlueprintCallable)
    void RestartLevel();

	void EliminatePlayersInEra(ETimelineEra Era);

protected:
    FTimerHandle RestartTimerHandle;

    FTimerHandle AlarmTimerHandlePast;
    FTimerHandle AlarmTimerHandleFuture;

    FTimerHandle PreAlarmTimerHandlePast;
    FTimerHandle PreAlarmTimerHandleFuture;

    UFUNCTION()
    void OnAlarmStarted(float AlarmEndTime, ETimelineEra Era);

    UFUNCTION()
    void OnAlarmCanceled();

    UFUNCTION()
    void OnPreAlarmStarted(float PreAlarmEndTime, AActor* PreAlarmInstigator, ETimelineEra Era);

    UFUNCTION()
    void OnPreAlarmCanceled();

    void PreAlarmTimeoutPast();
    void PreAlarmTimeoutFuture();

private:
    FString PendingMenuURL;
};