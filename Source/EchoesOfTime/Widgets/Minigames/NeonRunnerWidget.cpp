#include "NeonRunnerWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void UNeonRunnerWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (GameOverText) GameOverText->SetVisibility(ESlateVisibility::Hidden);
    if (ProgressText) ProgressText->SetVisibility(ESlateVisibility::Visible);

    if (GameCanvas)
        InitialChildCount = GameCanvas->GetChildrenCount();
    const int32 MaxSprites = 200; // more for tiles
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
                CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));
                CanvasSlot->SetZOrder(100);
            }
            SpritePool.Add(ImageWidget);
        }
    }
}

// --- TILE SYSTEM ---
// Add Tiles parameter
void UNeonRunnerWidget::DrawGameObjects(const FDinoPlayer& Player, const TArray<FDinoObstacle>& Obstacles, const TArray<FDinoObstacle>& Tiles, bool bGameOver, float SurvivalTime, float VictoryTime)
{
    if (!GameCanvas) return;

    FVector2D CanvasSize = GameCanvas->GetCachedGeometry().GetLocalSize();
    constexpr float ReferenceWidth = 1920.0f;
    constexpr float ReferenceHeight = 1080.0f;
    float ScaleX = CanvasSize.X / ReferenceWidth;
    float ScaleY = CanvasSize.Y / ReferenceHeight;
    float UniformScale = FMath::Min(ScaleX, ScaleY);

    CurrentSpriteIndex = 0;

    // --- TILE SYSTEM ---
    // Draw tiles first (behind everything)
    for (const FDinoObstacle& Tile : Tiles)
    {
        if (!Tile.bIsActive || Tile.Type != EObstacleType::Tile || !Tile.Texture) continue;
        FVector2D ScaledSize = Tile.Size * UniformScale;
        FVector2D ScaledPosition(Tile.Position.X * ScaleX, Tile.Position.Y * ScaleY);
        DrawSprite(ScaledPosition, Tile.Texture, ScaledSize);
    }

    // Player
    if (Player.Texture)
    {
        FVector2D ScaledSize = Player.Size * UniformScale;
        FVector2D ScaledPosition(Player.Position.X * ScaleX, Player.Position.Y * ScaleY);
        DrawSprite(ScaledPosition, Player.Texture, ScaledSize);
    }
    // Obstacles
    for (const FDinoObstacle& Obs : Obstacles)
    {
        if (!Obs.bIsActive || Obs.Type == EObstacleType::Gap || !Obs.Texture) continue;
        FVector2D ScaledSize = Obs.Size * UniformScale;
        FVector2D ScaledPosition(Obs.Position.X * ScaleX, Obs.Position.Y * ScaleY);
        DrawSprite(ScaledPosition, Obs.Texture, ScaledSize);
    }
    HideUnusedSprites();

    float ProgressRatio = FMath::Clamp(SurvivalTime / VictoryTime, 0.f, 1.f);
    int32 Percent = FMath::RoundToInt(ProgressRatio * 100.0f);
    if (ProgressText)
    {
        ProgressText->SetText(FText::FromString(FString::Printf(TEXT("Progress: %d%%"), Percent)));
    }
}

// No change needed below
UImage* UNeonRunnerWidget::GetOrCreateSpriteWidget()
{
    if (CurrentSpriteIndex < SpritePool.Num())
        return SpritePool[CurrentSpriteIndex++];
    UImage* Img = NewObject<UImage>(this);
    if (Img && GameCanvas)
    {
        Img->SetRenderOpacity(1.0f);
        Img->SetVisibility(ESlateVisibility::Hidden);
        UCanvasPanelSlot* CanvasSlot = GameCanvas->AddChildToCanvas(Img);
        if (CanvasSlot)
        {
            CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
            CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));
            CanvasSlot->SetZOrder(100);
        }
        SpritePool.Add(Img);
    }
    ++CurrentSpriteIndex;
    return Img;
}

void UNeonRunnerWidget::HideUnusedSprites()
{
    for (int32 i = CurrentSpriteIndex; i < SpritePool.Num(); ++i)
        if (SpritePool[i]) SpritePool[i]->SetVisibility(ESlateVisibility::Hidden);
}

void UNeonRunnerWidget::DrawSprite(FVector2D Position, UTexture2D* Texture, FVector2D Size)
{
    if (!GameCanvas || !Texture) return;
    UImage* Img = GetOrCreateSpriteWidget();
    if (!Img) return;
    Img->SetBrushFromTexture(Texture);
    Img->SetVisibility(ESlateVisibility::Visible);
    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Img->Slot))
    {
        FVector2D FinalPos = Position - (Size * 0.5f);
        CanvasSlot->SetPosition(FinalPos);
        CanvasSlot->SetSize(Size);
    }
}

void UNeonRunnerWidget::ShowGameOver()
{
    if (GameOverText)
    {
        GameOverText->SetText(FText::FromString(TEXT("GAME OVER!")));
        GameOverText->SetVisibility(ESlateVisibility::Visible);
    }
}

void UNeonRunnerWidget::ShowVictory()
{
    if (GameOverText)
    {
        GameOverText->SetText(FText::FromString(TEXT("VICTORY!")));
        GameOverText->SetVisibility(ESlateVisibility::Visible);
    }
}