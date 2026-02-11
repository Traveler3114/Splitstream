#include "DefaultGameInstance.h"
#include "Splitstream.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "TimerManager.h"

// --- User Settings / Input Mapping implementation (unchanged) ---

void UDefaultGameInstance::Init()
{
    Super::Init();
    LoadUserSettings();
}

void UDefaultGameInstance::LoadUserSettings()
{
    USaveGame* LoadedGame = UGameplayStatics::LoadGameFromSlot(TEXT("UserSettingsSave"), 0);
    UUserSettingsSaveGame* SaveGameInstance = Cast<UUserSettingsSaveGame>(LoadedGame);

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
                TArray<FEnhancedActionKeyMapping> OldMappings = RuntimeInputMappingContext->GetMappings();
                for (const FEnhancedActionKeyMapping& Mapping : OldMappings)
                {
                    if (Mapping.Action == Action)
                        RuntimeInputMappingContext->UnmapKey(Action, Mapping.Key);
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

// -------- Multiplayer/session logic below --------

void UDefaultGameInstance::HostLeaveToMainMenu(const FString& MainMenuMapPath)
{
    // Host is leaving: tell all clients (via GameMode or PlayerControllers), but here for explicitness.

    UWorld* World = GetWorld();
    if (!World)
        return;

    // Travel URL must be stored through async destroy session
    PendingMenuURL = MainMenuMapPath;

    // Tell all remote clients to leave to main menu (they will disconnect soon after)
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC && !PC->IsLocalController())
        {
            PC->ClientTravel(PendingMenuURL, TRAVEL_Absolute);
        }
    }
    // Show loading for host locally (nice UX)
    if (APlayerController* HostPC = UGameplayStatics::GetPlayerController(this, 0))
    {
        HostPC->ClientTravel(PendingMenuURL, TRAVEL_Absolute);
    }
    // Now actually destroy session and cleanup ports/sockets (after travel)
    RequestDestroySessionAndCleanup(true, PendingMenuURL);
}

void UDefaultGameInstance::RequestDestroySessionAndCleanup(bool bInTravelAfterDestroy, const FString& InPendingMenuURL)
{
    // Only ever do one destroy at a time
    if (bSessionDestroyInProgress)
        return;
    bSessionDestroyInProgress = true;

    PendingMenuURL = InPendingMenuURL;
    bTravelAfterSessionDestroy = bInTravelAfterDestroy;

    CleanupOSSDelegates(); // Always clear before rebinding
    IOnlineSessionPtr Session;
    if (IOnlineSubsystem* OSS = IOnlineSubsystem::Get())
        Session = OSS->GetSessionInterface();

    if (Session.IsValid())
    {
        OnDestroySessionCompleteDelegateHandle = Session->AddOnDestroySessionCompleteDelegate_Handle(
            FOnDestroySessionCompleteDelegate::CreateUObject(this, &UDefaultGameInstance::OnDestroySessionComplete));
        Session->DestroySession(NAME_GameSession);
    }
    else
    {
        // OSS is gone already: just clean netdriver and (maybe) travel
        CleanupNetDriver();
        if (bTravelAfterSessionDestroy && !PendingMenuURL.IsEmpty())
        {
            UWorld* World = GetWorld();
            if (World)
                World->ServerTravel(PendingMenuURL);
        }
        bSessionDestroyInProgress = false;
    }
}

void UDefaultGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    CleanupOSSDelegates();
    CleanupNetDriver();

    if (bTravelAfterSessionDestroy && !PendingMenuURL.IsEmpty())
    {
        UWorld* World = GetWorld();
        if (World)
            World->ServerTravel(PendingMenuURL);
    }
    bSessionDestroyInProgress = false;
}

void UDefaultGameInstance::CleanupOSSDelegates()
{
    IOnlineSessionPtr Session;
    if (IOnlineSubsystem* OSS = IOnlineSubsystem::Get())
        Session = OSS->GetSessionInterface();
    if (Session.IsValid() && OnDestroySessionCompleteDelegateHandle.IsValid())
    {
        Session->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
        OnDestroySessionCompleteDelegateHandle.Reset();
    }
}

void UDefaultGameInstance::CleanupNetDriver()
{
    UWorld* World = GetWorld();
    if (!World || !GEngine) return;
    FWorldContext* Context = GEngine->GetWorldContextFromWorld(World);
    if (Context)
    {
        for (FNamedNetDriver& NamedDriver : Context->ActiveNetDrivers)
        {
            if (NamedDriver.NetDriver)
            {
                UE_LOG(LogSplitstream, Warning, TEXT("Destroying leftover NetDriver: %s"), *NamedDriver.NetDriver->GetName());
                NamedDriver.NetDriver->SetWorld(nullptr);
                NamedDriver.NetDriver->Shutdown();
            }
        }
        Context->ActiveNetDrivers.Empty();
    }
}