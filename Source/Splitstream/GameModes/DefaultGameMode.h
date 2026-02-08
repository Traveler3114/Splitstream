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

protected:
	FTimerHandle RestartTimerHandle;
	FTimerHandle PreAlarmTimerHandle;

	UFUNCTION()
	void OnAlarmStarted(float AlarmEndTime, ETimelineEra Era);

	UFUNCTION()
	void OnAlarmCanceled();

	UFUNCTION()
	void OnPreAlarmStarted(float PreAlarmEndTime, AActor* PreAlarmInstigator, ETimelineEra Era);

	UFUNCTION()
	void OnPreAlarmCanceled();

	void PreAlarmTimeout();

private:
	FString PendingMenuURL;
};