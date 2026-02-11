#include "LobbyUI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Controllers/LobbyPlayerController.h"
#include "DefaultPlayerState.h"
#include "GameFramework/PlayerController.h"

void ULobbyUI::NativeConstruct()
{
    Super::NativeConstruct();

    if (ready_btn)      ready_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnReadyButtonClicked);
    if (changeteam_btn) changeteam_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnChangeTeamButtonClicked);
    if (start_btn)      start_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnStartButtonClicked);
    if (leave_btn)      leave_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnLeaveButtonClicked);

    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ADefaultPlayerState* DPS = PC->GetPlayerState<ADefaultPlayerState>())
        {
            DPS->OnReadyChanged.AddDynamic(this, &ULobbyUI::HandleLocalReadyChanged);
            DPS->OnTeamChanged.AddDynamic(this, &ULobbyUI::HandleLocalTeamChanged);
            RefreshReadyLabel(DPS->IsReady());
            RefreshTeamLabel(DPS->GetTeamTag());
        }
    }
}

void ULobbyUI::SetMapName(const FText& Name)
{
    if (MapName_txt)
    {
        MapName_txt->SetText(Name);
    }
}

void ULobbyUI::OnChangeTeamButtonClicked()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ADefaultPlayerState* DPS = PC->GetPlayerState<ADefaultPlayerState>())
        {
            // Determine next team tag (toggle between Past and Future; Solo logic is special)
            FGameplayTag CurrentTag = DPS->GetTeamTag();
            FGameplayTag PastTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Past"));
            FGameplayTag FutureTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Future"));

            FGameplayTag NewTag;
            if (CurrentTag == PastTag)
                NewTag = FutureTag;
            else
                NewTag = PastTag;

            if (PC->GetNetMode() == NM_Standalone || PC->HasAuthority())
            {
                DPS->SetTeamLocal(NewTag);
            }
            else
            {
                DPS->ServerSetTeam(NewTag);
            }

            RefreshTeamLabel(NewTag);
        }
    }
}

void ULobbyUI::RefreshTeamLabel(const FGameplayTag& TeamTag)
{
    if (team_txt)
    {
        // Display the "opposite" team, to indicate the destination of the toggle button
        FGameplayTag PastTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Past"));
        FGameplayTag FutureTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Future"));

        FString OppositeTeam;
        if (TeamTag == PastTag)
            OppositeTeam = TEXT("FUTURE");
        else if (TeamTag == FutureTag)
            OppositeTeam = TEXT("PAST");
        else
            OppositeTeam = TEXT("UNKNOWN");

        team_txt->SetText(FText::FromString(OppositeTeam));
    }
}

void ULobbyUI::HandleLocalTeamChanged(ADefaultPlayerState* PS)
{
    if (PS) RefreshTeamLabel(PS->GetTeamTag());
}

void ULobbyUI::SetStartButtonEnabled(bool bEnabled)
{
    if (start_btn) start_btn->SetIsEnabled(bEnabled);
}

void ULobbyUI::SetStartButtonVisibility(ESlateVisibility InVisibility)
{
    if (start_btn) start_btn->SetVisibility(InVisibility);
}

void ULobbyUI::OnLeaveButtonClicked()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ALobbyPlayerController* MyPC = Cast<ALobbyPlayerController>(PC))
        {
            MyPC->RequestLeaveToMainMenu();
        }
    }
}

void ULobbyUI::OnStartButtonClicked()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (PC->HasAuthority())
        {
            if (ALobbyPlayerController* LPC = Cast<ALobbyPlayerController>(PC))
            {
                LPC->ServerStartGame();
            }
        }
    }
}

void ULobbyUI::OnReadyButtonClicked()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ADefaultPlayerState* DPS = PC->GetPlayerState<ADefaultPlayerState>())
        {
            const bool NewState = !DPS->IsReady();

            if (PC->GetNetMode() == NM_Standalone || PC->HasAuthority())
            {
                DPS->SetReadyLocal(NewState);
            }
            else
            {
                DPS->ServerSetReady(NewState);
            }

            RefreshReadyLabel(NewState);
        }
    }
}

void ULobbyUI::RefreshReadyLabel(bool bIsReady)
{
    if (ready_txt)
    {
        ready_txt->SetText(bIsReady ? FText::FromString(TEXT("UNREADY"))
            : FText::FromString(TEXT("READY")));
    }
}

void ULobbyUI::HandleLocalReadyChanged(ADefaultPlayerState* PS)
{
    if (PS) RefreshReadyLabel(PS->IsReady());
}