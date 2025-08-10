// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerLobbyInfo.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Components/Button.h"

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