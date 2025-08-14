#include "PlayerLobbyInfo.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Components/Button.h"
#include "Components/SlateWrapperTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Controllers/LobbyPlayerController.h"

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

    // Find owning player controller (fallback to index 0)
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

void UPlayerLobbyInfo::SetTargetPlayerState(APlayerState* InTarget)
{
    TargetPlayerState = InTarget;
}