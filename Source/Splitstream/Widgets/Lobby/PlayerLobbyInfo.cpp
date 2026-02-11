#include "PlayerLobbyInfo.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Components/Button.h"
#include "Components/SlateWrapperTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Controllers/LobbyPlayerController.h"
#include "DefaultPlayerState.h"

void UPlayerLobbyInfo::NativeConstruct()
{
    Super::NativeConstruct();

    if (kick_btn)
    {
        kick_btn->OnClicked.AddDynamic(this, &UPlayerLobbyInfo::OnKickButtonClicked);
    }
}

void UPlayerLobbyInfo::OnKickButtonClicked()
{
    if (!TargetPlayerState) return;

    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    }
    if (!PC) return;

    if (ALobbyPlayerController* LPC = Cast<ALobbyPlayerController>(PC))
    {
        LPC->ServerKickPlayer(TargetPlayerState);
    }
}

void UPlayerLobbyInfo::SetPlayerName(const FText& Name)
{
    if (playername_txt)
    {
        playername_txt->SetText(Name);
    }
}

void UPlayerLobbyInfo::SetAvatarTexture(UTexture2D* Texture)
{
    if (!avatar_img) return;

    if (Texture)
    {
        avatar_img->SetBrushFromTexture(Texture, true);
        avatar_img->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        avatar_img->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UPlayerLobbyInfo::SetKickButtonVisible(bool bVisible)
{
    if (kick_btn)
    {
        kick_btn->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void UPlayerLobbyInfo::SetReadyState(bool bReady)
{
    if (ready_txt)
    {
        ready_txt->SetText(bReady ? FText::FromString(TEXT("Ready")) : FText::FromString(TEXT("Not Ready")));
    }
}

// Changed: Accept tag, display via display name
void UPlayerLobbyInfo::SetTeamState(const FGameplayTag& TeamTag)
{
    if (team_txt)
    {
        FString PrettyName = ADefaultPlayerState::GetTeamDisplayName(TeamTag);
        team_txt->SetText(FText::FromString(PrettyName));
    }
}

void UPlayerLobbyInfo::SetTargetPlayerState(APlayerState* InTarget)
{
    TargetPlayerState = InTarget;

    if (ADefaultPlayerState* DPS = Cast<ADefaultPlayerState>(TargetPlayerState))
    {
        SetTeamState(DPS->GetTeamTag());
    }
}