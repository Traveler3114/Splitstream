
#pragma once
#include "CoreMinimal.h"
#include "AdvancedFriendsGameInstance.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Saving/UserSettingsSaveGame.h"
#include "DefaultGameInstance.generated.h"

UCLASS()
class ECHOESOFTIME_API UDefaultGameInstance : public UAdvancedFriendsGameInstance
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintImplementableEvent, Category = "GameInstance")
    void CreateSession(const FString& LevelName, const TSoftObjectPtr<UWorld>& Level);
    // Runtime mapping context (duplicated from default asset and merged with saved changes)
    UPROPERTY()
    UInputMappingContext* RuntimeInputMappingContext = nullptr;

    // Used to reconstruct InputMappingContext runtime on load/save
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext = nullptr; // (Assign this in editor or constructor!)

    // List of all InputActions you want to expose to the UI and bind (populate in editor)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TArray<UInputAction*> AllInputActions;

    UPROPERTY()
    float MouseSensitivity = 1.0f;

    virtual void Init() override;

    void LoadUserSettings();
    void SaveUserSettings(const TArray<FSavedKeybind>& Keybinds, float NewMouseSensitivity);

    // Helpers
    UInputAction* FindInputActionByName(const FName& ActionName) const;
    UInputMappingContext* GetCurrentInputMappingContext() const { return RuntimeInputMappingContext ? RuntimeInputMappingContext : DefaultMappingContext; }
    float GetMouseSensitivity() const { return MouseSensitivity; }
};