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

	UFUNCTION(BlueprintCallable)
	void HostLeaveLobby();

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
	struct FDelegateHandleWrapper { FDelegateHandle Handle; bool bBound = false; };
	FDelegateHandleWrapper DestroySessionCompleteHandle;
	bool bDestroyingSession = false;
	FTimerHandle LeaveTimerHandle;
	FString PendingMenuURL;

	UFUNCTION()
	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void DoServerTravelToMenu();
};