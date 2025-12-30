#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Minigames/NeonRunnerMiniGame.h"
#include "NeonRunnerWidget.generated.h"

UCLASS()
class ECHOESOFTIME_API UNeonRunnerWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget)) UCanvasPanel* GameCanvas;
    UPROPERTY(meta = (BindWidget)) UTextBlock* GameOverText;
    UPROPERTY(meta = (BindWidget)) UTextBlock* ProgressText;
    UPROPERTY(meta = (BindWidget)) UProgressBar* HoverProgressBar;

    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void DrawGameObjects(const FNeonRunnerPlayer& Player, const TArray<FNeonRunnerObstacle>& Obstacles, const TArray<FNeonRunnerObstacle>& Tiles, bool bGameOver, float SurvivalTime, float VictoryTime, float HoverFraction);

    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void ShowGameOver();

    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void ShowVictory();

private:
    void DrawSprite(FVector2D Position, UTexture2D* Texture, FVector2D Size);
    UImage* GetOrCreateSpriteWidget();
    void HideUnusedSprites();

    UPROPERTY() TArray<UImage*> SpritePool;
    int32 CurrentSpriteIndex = 0;
    int32 InitialChildCount = 0;
};