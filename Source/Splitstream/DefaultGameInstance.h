#pragma once

#include "CoreMinimal.h"
#include "AdvancedFriendsGameInstance.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Saving/UserSettingsSaveGame.h"
#include "DefaultGameInstance.generated.h"

UCLASS()
class SPLITSTREAM_API UDefaultGameInstance : public UAdvancedFriendsGameInstance
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintImplementableEvent, Category = "GameInstance")
    void CreateSession(const FString& LevelName, const TSoftObjectPtr<UWorld>& Level, const TSoftObjectPtr<UWorld>& LobbyLevel);

    UPROPERTY()
    UInputMappingContext* RuntimeInputMappingContext = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TArray<UInputAction*> AllInputActions;

    UPROPERTY()
    float MouseSensitivity = 1.0f;

    virtual void Init() override;

    void LoadUserSettings();
    void SaveUserSettings(const TArray<FSavedKeybind>& Keybinds, float NewMouseSensitivity);

    UInputAction* FindInputActionByName(const FName& ActionName) const;
    UInputMappingContext* GetCurrentInputMappingContext() const { return RuntimeInputMappingContext ? RuntimeInputMappingContext : DefaultMappingContext; }
    float GetMouseSensitivity() const { return MouseSensitivity; }

    // ---- Multiplayer/session-centric logic ----
    UFUNCTION()
    void HostLeaveToMainMenu(const FString& MainMenuMapPath = TEXT("/Game/Maps/MainMenuMap"));

    UFUNCTION()
    void RequestDestroySessionAndCleanup(bool bTravelAfterDestroy, const FString& InPendingMenuURL);

    UFUNCTION()
    void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

    UFUNCTION()
    void CleanupNetDriver();
    UFUNCTION()
    void CleanupOSSDelegates();

private:
    FString PendingMenuURL;
    bool bTravelAfterSessionDestroy = false;
    FDelegateHandle OnDestroySessionCompleteDelegateHandle;
    bool bSessionDestroyInProgress = false;
};