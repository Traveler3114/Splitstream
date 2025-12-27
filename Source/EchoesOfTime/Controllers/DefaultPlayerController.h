// DefaultPlayerController.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DefaultPlayerController.generated.h"

class UPauseMenuWidget;
class ATerminal;

UCLASS()
class ECHOESOFTIME_API ADefaultPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    ADefaultPlayerController();

    virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnPossess(APawn* InPawn) override;
    virtual void SetupInputComponent() override;
    virtual void OnRep_PlayerState() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
	void SetupOverlay();

    void TogglePauseMenu();

    void BindAttributeDelegates();
    void BindGameplayTagDelegates();

    void OnHealthChanged(const struct FOnAttributeChangeData& Data);
    void OnIllegalTagChanged(const struct FGameplayTag Tag, int32 NewCount);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UPauseMenuWidget> PauseMenuWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UCalendarWidget> CalendarWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UUserWidget> LoadingWidgetClass;

    UPROPERTY()
    UUserWidget* LoadingWidgetInstance = nullptr;

    UPROPERTY(BlueprintReadWrite)
    UCalendarWidget* CalendarWidgetInstance;

    bool bIsPauseMenuOpen = false;

    UFUNCTION(Client, Reliable)
    void ClientShowCalendarWidget(const TArray<FCalendarDateRecord>& CivilianDateRecords);

    UFUNCTION(Client, Reliable, BlueprintCallable)
    void ClientUpdateDetectionWidget(AActor* DetectorActor, float Progress, bool bIsLocked = false, float AngleDegrees = 0.0f);

    UFUNCTION(Client, Reliable)
    void ClientUpdateDetectionActorWidget(AActor* DetectorActor, float Progress, bool bIsLocked);

    UFUNCTION(Client, Reliable)
    void ClientShowLoadingScreen();

    UFUNCTION()
    void OnMoneyCollectedChanged(int32 Current, int32 Target);

    UFUNCTION()
    void HandleRepairETAStarted(class ARepairableBase* Repairable, float Duration);

	UFUNCTION(Server, Reliable)
    void ServerExecuteAction(UObject* Target, const FServerActionPayload& Payload);
private:
    UPROPERTY()
    class ACharacterHUD* CharacterHUD;

    UPROPERTY()
    UPauseMenuWidget* PauseMenuWidget;

    FDelegateHandle IllegalTagDelegateHandle;

    UFUNCTION()
    void HandlePauseMenuResumed();

    FTimerHandle PingUpdateTimerHandle;
    void UpdatePingOnOverlay();

    UPROPERTY()
    float AlarmEndTime = 0.f;

    FTimerHandle AlarmUpdateTimerHandle;

    UFUNCTION()
    void HandleAlarmStarted(float InAlarmEndTime);

    UFUNCTION()
    void HandleAlarmCanceled();

    void UpdateAlarmUI();

    UPROPERTY()
    float PreAlarmEndTime = 0.f;

    FTimerHandle PreAlarmUpdateTimerHandle;

    UFUNCTION()
    void HandlePreAlarmStarted(float InPreAlarmEndTime, AActor* PreAlarmInstigator);

    UFUNCTION()
    void HandlePreAlarmCanceled();

    void UpdatePreAlarmUI();

    FTimerHandle RepairCountdownsUpdateTimer;
    TMap<AActor*, float> ActiveRepairCountdowns;

    void UpdateRepairCountdownUI();
};