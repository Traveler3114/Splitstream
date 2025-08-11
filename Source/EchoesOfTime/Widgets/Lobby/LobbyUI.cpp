#include "LobbyUI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Controllers/LobbyPlayerController.h"
#include "GameModes/LobbyGameMode.h"

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

void ULobbyUI::OnChangeButtonClicked()
{
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
        PC->ServerSetTeamTag(NewTag);
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
        PC->ServerSetReadyState(bLocalReady);
    }
}