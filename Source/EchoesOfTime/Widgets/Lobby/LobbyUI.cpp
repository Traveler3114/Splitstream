#include "LobbyUI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Controllers/LobbyPlayerController.h"
#include "GameModes/LobbyGameMode.h"
#include "GameStates/LobbyGameState.h"
#include "DefaultPlayerState.h"
#include "Engine/World.h"

void ULobbyUI::NativeConstruct()
{
    Super::NativeConstruct();
    if (ready_btn)
    {
        ready_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnReadyButtonClicked);
    }

    if (changeteam_btn)
    {
        changeteam_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnChangeButtonClicked);
    }
    if (start_btn)
    {
        start_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnStartButtonClicked);
    }
    if (leave_btn)
    {
        leave_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnLeaveButtonClicked);
    }

    // Bind to GameState events for reactive UI updates
    BindToGameStateEvents();
}

void ULobbyUI::NativeDestruct()
{
    UnbindFromGameStateEvents();
    Super::NativeDestruct();
}

void ULobbyUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    // Ensure GameState binding in case it wasn't available during NativeConstruct
    if (!CachedLobbyGameState && GetWorld())
    {
        BindToGameStateEvents();
    }
}

void ULobbyUI::OnLeaveButtonClicked()
{
    if (ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(GetOwningPlayer()))
    {
        PC->ServerRequestLeaveLobby();
    }
}


void ULobbyUI::OnStartButtonClicked()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ALobbyGameMode* GM = Cast<ALobbyGameMode>(PC->GetWorld()->GetAuthGameMode()))
        {
            GM->StartGame();
        }
    }
}


void ULobbyUI::SetStartButtonEnabled(bool bEnabled)
{
    if (start_btn)
    {
        start_btn->SetIsEnabled(bEnabled);
    }
}

void ULobbyUI::SetStartButtonVisibility(ESlateVisibility InVisibility)
{
    if (start_btn)
    {
        start_btn->SetVisibility(InVisibility);
    }
}

void ULobbyUI::OnReadyButtonClicked()
{
    // Check if ready toggle is allowed based on lobby phase
    if (CachedLobbyGameState && !CachedLobbyGameState->CanPlayersToggleReady())
    {
        UE_LOG(LogLobby, Warning, TEXT("Ready toggle not allowed in current lobby phase"));
        return;
    }

    bLocalReady = !bLocalReady;

    if (ready_btn)
    {
        UTextBlock* ReadyText = Cast<UTextBlock>(ready_btn->GetChildAt(0));
        if (ReadyText)
        {
            ReadyText->SetText(bLocalReady ? FText::FromString(TEXT("Unready")) : FText::FromString(TEXT("Ready")));
        }
    }

    if (ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(GetOwningPlayer()))
    {
        ADefaultPlayerState* PlayerState = Cast<ADefaultPlayerState>(PC ? PC->PlayerState : nullptr);
        if (PlayerState)
        {
            PlayerState->ServerSetReadyState(bLocalReady);
        }
    }
}

void ULobbyUI::OnChangeButtonClicked()
{
    // Check if team changes are allowed based on lobby phase
    if (CachedLobbyGameState && !CachedLobbyGameState->CanPlayersChangeTeams())
    {
        UE_LOG(LogLobby, Warning, TEXT("Team changes not allowed in current lobby phase"));
        return;
    }

    const FString CurrentText = team_txt->GetText().ToString();
    FGameplayTag NewTag;
    if (CurrentText.Equals(TEXT("Future"), ESearchCase::IgnoreCase))
    {
        team_txt->SetText(FText::FromString(TEXT("Past")));
        NewTag = FGameplayTag::RequestGameplayTag(FName("Team.Past"));
    }
    else
    {
        team_txt->SetText(FText::FromString(TEXT("Future")));
        NewTag = FGameplayTag::RequestGameplayTag(FName("Team.Future"));
    }
    if (ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(GetOwningPlayer()))
    {
        ADefaultPlayerState* PlayerState = Cast<ADefaultPlayerState>(PC ? PC->PlayerState : nullptr);
        if (PlayerState)
        {
            PlayerState->ServerSetTeamTag(NewTag);
        }
    }
}

void ULobbyUI::OnReadyButtonClicked()
{
    // Check if ready toggle is allowed based on lobby phase
    if (CachedLobbyGameState && !CachedLobbyGameState->CanPlayersToggleReady())
    {
        UE_LOG(LogLobby, Warning, TEXT("Ready toggle not allowed in current lobby phase"));
        return;
    }

    bLocalReady = !bLocalReady;

    if (ready_btn)
    {
        UTextBlock* ReadyText = Cast<UTextBlock>(ready_btn->GetChildAt(0));
        if (ReadyText)
        {
            ReadyText->SetText(bLocalReady ? FText::FromString(TEXT("Unready")) : FText::FromString(TEXT("Ready")));
        }
    }

    if (ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(GetOwningPlayer()))
    {
        ADefaultPlayerState* PlayerState = Cast<ADefaultPlayerState>(PC ? PC->PlayerState : nullptr);
        if (PlayerState)
        {
            PlayerState->ServerSetReadyState(bLocalReady);
        }
    }
}

// NEW: GameState event handlers
void ULobbyUI::OnLobbyPhaseChanged(ELobbyPhase NewPhase)
{
    UE_LOG(LogLobby, Log, TEXT("UI: Lobby phase changed to %d"), (int32)NewPhase);
    UpdateUIForPhase(NewPhase);
}

void ULobbyUI::OnCountdownUpdate(float TimeRemaining)
{
    // TODO: Update countdown display in UI
    UE_LOG(LogLobby, VeryVerbose, TEXT("UI: Countdown: %.1f seconds"), TimeRemaining);
}

void ULobbyUI::OnPlayerRosterChanged()
{
    UE_LOG(LogLobby, Log, TEXT("UI: Player roster changed"));
    // TODO: Refresh player list display
}

void ULobbyUI::OnAllPlayersReadyChanged()
{
    UE_LOG(LogLobby, Log, TEXT("UI: All players ready state changed"));
    // The start button enabling is handled via the existing ClientSetStartButtonEnabled RPC
    // This event can be used for additional UI feedback
}

void ULobbyUI::BindToGameStateEvents()
{
    if (GetWorld())
    {
        CachedLobbyGameState = GetWorld()->GetGameState<ALobbyGameState>();
        if (CachedLobbyGameState)
        {
            // Note: Since these are Blueprint implementable events, we'd need to bind in Blueprint
            // or convert them to multicast delegates. For now, keeping as stub for architecture.
            UE_LOG(LogLobby, Log, TEXT("UI bound to LobbyGameState events"));
        }
    }
}

void ULobbyUI::UnbindFromGameStateEvents()
{
    if (CachedLobbyGameState)
    {
        // Unbind from events if needed
        CachedLobbyGameState = nullptr;
    }
}

void ULobbyUI::UpdateUIForPhase(ELobbyPhase Phase)
{
    switch (Phase)
    {
        case ELobbyPhase::Gathering:
            // Enable team change and ready buttons
            if (changeteam_btn) changeteam_btn->SetIsEnabled(true);
            if (ready_btn) ready_btn->SetIsEnabled(true);
            break;
            
        case ELobbyPhase::Countdown:
            // Keep ready button enabled but team change disabled
            if (changeteam_btn) changeteam_btn->SetIsEnabled(false);
            if (ready_btn) ready_btn->SetIsEnabled(true);
            break;
            
        case ELobbyPhase::Locked:
        case ELobbyPhase::Traveling:
            // Disable all player actions
            if (changeteam_btn) changeteam_btn->SetIsEnabled(false);
            if (ready_btn) ready_btn->SetIsEnabled(false);
            break;
            
        default:
            break;
    }
}