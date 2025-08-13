#include "PlayerLobbyInfo.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Components/Button.h"
#include "Components/SlateWrapperTypes.h"
#include "GameplayTagContainer.h"
#include "GameStates/LobbyGameState.h"




void UPlayerLobbyInfo::NativeConstruct()
{
    Super::NativeConstruct();

    if (kick_btn)
    {
        kick_btn->OnClicked.AddDynamic(this, &UPlayerLobbyInfo::HandleKickClicked);
    }
}

void UPlayerLobbyInfo::HandleKickClicked()
{
    OnKickRequested.Broadcast();
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
    if (avatar_img && Texture)
    {
        FSlateBrush Brush;
        Brush.SetResourceObject(Texture);
        Brush.ImageSize = FVector2D(Texture->GetSizeX(), Texture->GetSizeY());
        avatar_img->SetBrush(Brush);
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

// PlayerLobbyInfo.cpp
void UPlayerLobbyInfo::SetTeamTag(FGameplayTag NewTag)
{
    if (team_txt)
    {
        // Get only the last part of the tag (e.g., "Future" from "Team.Future")
        FString Leaf = NewTag.GetTagLeafName().ToString();
        team_txt->SetText(FText::FromString(Leaf));
    }
}

// NEW: Unified method for batch UI updates - preferred over individual setters
void UPlayerLobbyInfo::ApplyLobbyPlayerViewData(const FLobbyPlayerViewData& ViewData)
{
    // Update all fields at once to reduce individual calls
    SetPlayerName(ViewData.DisplayName);
    SetReadyState(ViewData.bReady);
    SetTeamTag(ViewData.TeamTag);
    SetKickButtonVisible(ViewData.bCanKick);
    
    // TODO: Future avatar loading based on ViewData.AvatarId
    // When async avatar system is implemented, this will fetch avatar by ID
    // For now, keep existing avatar texture logic in DefaultPlayerState
    
    // TODO: Future role display based on ViewData.RoleTag  
    // When role selection UI is implemented, display role in separate UI element
    // Reserved for future role selection and specialization system
    
    // TODO: Future analytics integration
    // Track UI update patterns and performance metrics
    // Log user interaction patterns for lobby optimization
}