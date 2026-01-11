#pragma once

#include "CoreMinimal.h"
#include "BaseGameMode.h"
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
	void OnAlarmStarted(float AlarmEndTime);

	UFUNCTION()
	void OnAlarmCanceled();

	UFUNCTION()
	void OnPreAlarmStarted(float PreAlarmEndTime, AActor* PreAlarmInstigator);

	UFUNCTION()
	void OnPreAlarmCanceled();

	void PreAlarmTimeout();

private:
	FString PendingMenuURL;
};