#include "FirewallWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void UFirewallWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (GameOverText)
        GameOverText->SetVisibility(ESlateVisibility::Hidden);

    if (BossHPText)
        BossHPText->SetVisibility(ESlateVisibility::Hidden);

    if (GameCanvas)
        InitialChildCount = GameCanvas->GetChildrenCount();

    const int32 MaxSprites = 250;
    SpritePool.Reserve(MaxSprites);

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

void UFirewallWidget::SetLives(int32 NewLives)
{
    if (LivesText)
        LivesText->SetText(FText::FromString(FString::Printf(TEXT("Lives: %d"), NewLives)));
}

void UFirewallWidget::SetBossHP(int32 BossHP, int32 MaxHP)
{
    if (BossHPText)
    {
        if (BossHP < 0)
            BossHPText->SetVisibility(ESlateVisibility::Hidden);
        else
        {
            BossHPText->SetVisibility(ESlateVisibility::Visible);
            BossHPText->SetText(FText::FromString(FString::Printf(TEXT("Boss: %d / %d"), BossHP, MaxHP)));
        }
    }
}

void UFirewallWidget::DrawGameObjects(
    const FMiniGamePlayer& Player,
    const TArray<FMiniGameEnemy>& Enemies,
    const TArray<FMiniGameHeavyEnemy>& HeavyEnemies,
    const TArray<FMiniGameProjectile>& Projectiles,
    const TArray<FMiniGameEnemyBullet>& EnemyBullets,
    const TArray<FMiniGameHeavyEnemyBullet>& HeavyEnemyBullets,
    bool bHasBoss,
    const FMiniGameBoss& Boss
)
{
    if (!GameCanvas)
        return;

    FVector2D CanvasSize = GameCanvas->GetCachedGeometry().GetLocalSize();
    constexpr float ReferenceWidth = 1920.0f;
    constexpr float ReferenceHeight = 1080.0f;
    float ScaleX = CanvasSize.X / ReferenceWidth;
    float ScaleY = CanvasSize.Y / ReferenceHeight;
    float UniformScale = FMath::Min(ScaleX, ScaleY);

    CurrentSpriteIndex = 0;

    // Draw player
    if (Player.Texture)
    {
        FVector2D ScaledSize = Player.Size * UniformScale;
        FVector2D ScaledPosition(Player.Position.X * ScaleX, Player.Position.Y * ScaleY);
        DrawSprite(ScaledPosition, Player.Texture, ScaledSize);
    }

    // Draw boss (if present and has valid texture)
    if (bHasBoss && Boss.Texture)
    {
        FVector2D ScaledSize = Boss.Size * UniformScale;
        FVector2D ScaledPosition(Boss.Position.X * ScaleX, Boss.Position.Y * ScaleY);
        DrawSprite(ScaledPosition, Boss.Texture, ScaledSize);
    }

    for (const FMiniGameEnemy& Enemy : Enemies)
        if (Enemy.bIsAlive && Enemy.Texture)
            DrawSprite(FVector2D(Enemy.Position.X * ScaleX, Enemy.Position.Y * ScaleY), Enemy.Texture, Enemy.Size * UniformScale);

    for (const FMiniGameHeavyEnemy& Enemy : HeavyEnemies)
        if (Enemy.bIsAlive && Enemy.Texture)
            DrawSprite(FVector2D(Enemy.Position.X * ScaleX, Enemy.Position.Y * ScaleY), Enemy.Texture, Enemy.Size * UniformScale);

    for (const FMiniGameProjectile& Projectile : Projectiles)
        if (Projectile.bIsActive && Projectile.Texture)
            DrawSprite(FVector2D(Projectile.Position.X * ScaleX, Projectile.Position.Y * ScaleY), Projectile.Texture, Projectile.Size * UniformScale);

    for (const FMiniGameEnemyBullet& Bullet : EnemyBullets)
        if (Bullet.bIsActive && Bullet.Texture)
            DrawSprite(FVector2D(Bullet.Position.X * ScaleX, Bullet.Position.Y * ScaleY), Bullet.Texture, Bullet.Size * UniformScale);

    for (const FMiniGameHeavyEnemyBullet& Bullet : HeavyEnemyBullets)
        if (Bullet.bIsActive && Bullet.Texture)
            DrawSprite(FVector2D(Bullet.Position.X * ScaleX, Bullet.Position.Y * ScaleY), Bullet.Texture, Bullet.Size * UniformScale);

    HideUnusedSprites();
}

UImage* UFirewallWidget::GetOrCreateSpriteWidget()
{
    if (CurrentSpriteIndex < SpritePool.Num())
        return SpritePool[CurrentSpriteIndex++];
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
        if (SpritePool[i])
            SpritePool[i]->SetVisibility(ESlateVisibility::Hidden);
}

void UFirewallWidget::DrawSprite(FVector2D Position, UTexture2D* Texture, FVector2D Size)
{
    if (!GameCanvas || !Texture)
        return;

    UImage* ImageWidget = GetOrCreateSpriteWidget();
    if (!ImageWidget)
        return;

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