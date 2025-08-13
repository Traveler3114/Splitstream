#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyUI.generated.h"

class UButton;
class UTextBlock;
class ALobbyGameState;

UCLASS()
class ECHOESOFTIME_API ULobbyUI : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    void SetStartButtonEnabled(bool bEnabled);

    void SetStartButtonVisibility(ESlateVisibility InVisibility);

    UFUNCTION()
    void OnChangeButtonClicked();

    UFUNCTION()
    void OnStartButtonClicked();

    UFUNCTION()
    void OnLeaveButtonClicked();

    UPROPERTY(meta = (BindWidget))
    UButton* start_btn;

    UPROPERTY(meta = (BindWidget))
    UButton* leave_btn;

    UPROPERTY(meta = (BindWidget))
    UButton* ready_btn;

    UPROPERTY(meta = (BindWidget))
    UButton* changeteam_btn;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* team_txt;

    UFUNCTION()
    void OnReadyButtonClicked();

    // NEW: Listen for GameState changes
    UFUNCTION()
    void OnLobbyPhaseChanged(ELobbyPhase NewPhase);

    UFUNCTION()
    void OnCountdownUpdate(float TimeRemaining);

    UFUNCTION()
    void OnPlayerRosterChanged();

    UFUNCTION()
    void OnAllPlayersReadyChanged();

protected:
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    bool bLocalReady = false;

    // Reference to GameState for listening to changes
    UPROPERTY()
    ALobbyGameState* CachedLobbyGameState = nullptr;

    // Internal helper to bind to GameState events
    void BindToGameStateEvents();
    void UnbindFromGameStateEvents();

    // Helper to update UI based on lobby phase
    void UpdateUIForPhase(ELobbyPhase Phase);
};