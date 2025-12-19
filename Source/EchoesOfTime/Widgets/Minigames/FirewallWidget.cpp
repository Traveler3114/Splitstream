// Copyright Epic Games, Inc.  All Rights Reserved.

#include "FirewallWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void UFirewallWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Only log major events, not per-frame or per-sprite.
    if (GameOverText)
    {
        GameOverText->SetVisibility(ESlateVisibility::Hidden);
    }

    if (GameCanvas)
    {
        InitialChildCount = GameCanvas->GetChildrenCount();
    }

    // Preallocate pool to expected max count for all sprites (enemies + player + projectiles + extra)
    const int32 MaxSprites = 100;
    SpritePool.Reserve(MaxSprites);

    // Prepopulate sprite pool and add them to GameCanvas (hidden by default)
    for (int32 i = SpritePool.Num(); i < MaxSprites; ++i)
    {
        UImage* ImageWidget = NewObject<UImage>(this);
        if (ImageWidget && GameCanvas)
        {
            ImageWidget->SetRenderOpacity(1.0f);
            ImageWidget->SetVisibility(ESlateVisibility::Hidden);

            UCanvasPanelSlot* CanvasSlot = GameCanvas->AddChildToCanvas(ImageWidget);
            if (CanvasSlot)
            {
                CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
                CanvasSlot->SetAlignment(FVector2D(0.0f, 0.0f));
                CanvasSlot->SetZOrder(100);
            }
            SpritePool.Add(ImageWidget);
        }
    }
}

void UFirewallWidget::SetScore(int32 NewScore)
{
    if (ScoreText)
    {
        ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), NewScore)));
    }
}

void UFirewallWidget::SetLives(int32 NewLives)
{
    if (LivesText)
    {
        LivesText->SetText(FText::FromString(FString::Printf(TEXT("Lives: %d"), NewLives)));
    }
}

void UFirewallWidget::DrawGameObjects(const FMiniGamePlayer& Player, const TArray<FMiniGameEnemy>& Enemies, const TArray<FMiniGameProjectile>& Projectiles)
{
    if (!GameCanvas)
        return;

    FVector2D CanvasSize = GameCanvas->GetCachedGeometry().GetLocalSize();

    // Reference resolution—change as needed to match your game's design baseline
    constexpr float ReferenceWidth = 1920.0f;
    constexpr float ReferenceHeight = 1080.0f;

    // Scale factors
    float ScaleX = CanvasSize.X / ReferenceWidth;
    float ScaleY = CanvasSize.Y / ReferenceHeight;
    // Uniform scale to maintain proportions (optional, but recommended for pixel-art/squares)
    float UniformScale = FMath::Min(ScaleX, ScaleY);

    CurrentSpriteIndex = 0;

    // Draw player
    if (Player.Texture)
    {
        FVector2D ScaledSize = Player.Size * UniformScale;
        // Scale position as well so position is relative to canvas
        FVector2D ScaledPosition(
            Player.Position.X * ScaleX,
            Player.Position.Y * ScaleY
        );
        DrawSprite(ScaledPosition, Player.Texture, ScaledSize);
    }

    // Draw enemies
    for (const FMiniGameEnemy& Enemy : Enemies)
    {
        if (Enemy.bIsAlive && Enemy.Texture)
        {
            FVector2D ScaledSize = Enemy.Size * UniformScale;
            FVector2D ScaledPosition(
                Enemy.Position.X * ScaleX,
                Enemy.Position.Y * ScaleY
            );
            DrawSprite(ScaledPosition, Enemy.Texture, ScaledSize);
        }
    }

    // Draw projectiles
    for (const FMiniGameProjectile& Projectile : Projectiles)
    {
        if (Projectile.bIsActive && Projectile.Texture)
        {
            FVector2D ScaledSize = Projectile.Size * UniformScale;
            FVector2D ScaledPosition(
                Projectile.Position.X * ScaleX,
                Projectile.Position.Y * ScaleY
            );
            DrawSprite(ScaledPosition, Projectile.Texture, ScaledSize);
        }
    }

    HideUnusedSprites();
}

UImage* UFirewallWidget::GetOrCreateSpriteWidget()
{
    // Never log in per-frame sprite get.
    if (CurrentSpriteIndex < SpritePool.Num())
    {
        return SpritePool[CurrentSpriteIndex++];
    }
    // If pool exhausted (should not happen if you preallocated enough), add one more.
    UImage* ImageWidget = NewObject<UImage>(this);
    if (ImageWidget && GameCanvas)
    {
        ImageWidget->SetRenderOpacity(1.0f);
        ImageWidget->SetVisibility(ESlateVisibility::Hidden);

        UCanvasPanelSlot* CanvasSlot = GameCanvas->AddChildToCanvas(ImageWidget);
        if (CanvasSlot)
        {
            CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
            CanvasSlot->SetAlignment(FVector2D(0.0f, 0.0f));
            CanvasSlot->SetZOrder(100);
        }
        SpritePool.Add(ImageWidget);
    }
    ++CurrentSpriteIndex;
    return ImageWidget;
}

void UFirewallWidget::HideUnusedSprites()
{
    for (int32 i = CurrentSpriteIndex; i < SpritePool.Num(); ++i)
    {
        if (SpritePool[i])
        {
            SpritePool[i]->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

void UFirewallWidget::DrawSprite(FVector2D Position, UTexture2D* Texture, FVector2D Size)
{
    if (!GameCanvas || !Texture)
    {
        return;
    }

    UImage* ImageWidget = GetOrCreateSpriteWidget();
    if (!ImageWidget)
    {
        return;
    }

    ImageWidget->SetBrushFromTexture(Texture);
    ImageWidget->SetVisibility(ESlateVisibility::Visible);

    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ImageWidget->Slot))
    {
        FVector2D FinalPosition = Position - (Size * 0.5f);
        CanvasSlot->SetPosition(FinalPosition);
        CanvasSlot->SetSize(Size);
    }
}

void UFirewallWidget::ShowGameOver()
{
    if (GameOverText)
    {
        GameOverText->SetText(FText::FromString(TEXT("GAME OVER!")));
        GameOverText->SetVisibility(ESlateVisibility::Visible);
    }
}

void UFirewallWidget::ShowVictory()
{
    if (GameOverText)
    {
        GameOverText->SetText(FText::FromString(TEXT("VICTORY!")));
        GameOverText->SetVisibility(ESlateVisibility::Visible);
    }
}