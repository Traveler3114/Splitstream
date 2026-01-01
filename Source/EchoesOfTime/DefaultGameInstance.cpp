// Fill out your copyright notice in the Description page of Project Settings.

#include "DefaultGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UDefaultGameInstance::Init()
{
    Super::Init();
    LoadUserSettings();
}

void UDefaultGameInstance::LoadUserSettings()
{
    USaveGame* LoadedGame = UGameplayStatics::LoadGameFromSlot(TEXT("UserSettingsSave"), 0);
    UUserSettingsSaveGame* SaveGameInstance = Cast<UUserSettingsSaveGame>(LoadedGame);

    // Always duplicate to get a fresh context (never mutate asset!)
    RuntimeInputMappingContext = nullptr;
    MouseSensitivity = 1.0f;
    if (DefaultMappingContext)
        RuntimeInputMappingContext = DuplicateObject<UInputMappingContext>(DefaultMappingContext, this);

    if (SaveGameInstance && RuntimeInputMappingContext)
    {
        for (const FSavedKeybind& Saved : SaveGameInstance->SavedKeybinds)
        {
            UInputAction* Action = FindInputActionByName(Saved.ActionName);
            if (Action)
            {
                // Remove all existing keys for this action
                TArray<FEnhancedActionKeyMapping> OldMappings = RuntimeInputMappingContext->GetMappings();
                for (const FEnhancedActionKeyMapping& Mapping : OldMappings)
                {
                    if (Mapping.Action == Action)
                    {
                        RuntimeInputMappingContext->UnmapKey(Action, Mapping.Key);
                    }
                }
                RuntimeInputMappingContext->MapKey(Action, Saved.Key);
            }
        }
        MouseSensitivity = SaveGameInstance->MouseSensitivity;
    }
}

void UDefaultGameInstance::SaveUserSettings(const TArray<FSavedKeybind>& Keybinds, float NewMouseSensitivity)
{
    UUserSettingsSaveGame* SaveGameInstance = Cast<UUserSettingsSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UUserSettingsSaveGame::StaticClass()));
    if (!SaveGameInstance)
        return;
    SaveGameInstance->SavedKeybinds = Keybinds;
    SaveGameInstance->MouseSensitivity = NewMouseSensitivity;
    UGameplayStatics::SaveGameToSlot(SaveGameInstance, TEXT("UserSettingsSave"), 0);

    // Update runtime copy immediately
    LoadUserSettings();
}

UInputAction* UDefaultGameInstance::FindInputActionByName(const FName& ActionName) const
{
    for (UInputAction* Action : AllInputActions)
    {
        if (Action && Action->GetFName() == ActionName)
            return Action;
    }
    return nullptr;
}