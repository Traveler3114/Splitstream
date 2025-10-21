// DefaultGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "DefaultGameMode.generated.h"

UCLASS()
class ECHOESOFTIME_API ADefaultGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	UFUNCTION(BlueprintCallable)
	void RestartLevel();

protected:
	FTimerHandle RestartTimerHandle;

	// Called on server when alarm starts (bound to GameState->OnAlarmStarted)
	UFUNCTION()
	void OnAlarmStarted(float AlarmEndTime);

	// Called on server when alarm is canceled (bound to GameState->OnAlarmCanceled)
	UFUNCTION()
	void OnAlarmCanceled();
};