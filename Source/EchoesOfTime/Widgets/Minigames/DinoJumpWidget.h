#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Minigames/DinoJumpMiniGame.h"
#include "DinoJumpWidget.generated.h"

UCLASS()
class ECHOESOFTIME_API UDinoJumpWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget)) UCanvasPanel* GameCanvas;
	UPROPERTY(meta = (BindWidget)) UTextBlock* GameOverText;
	UPROPERTY(meta = (BindWidget)) UTextBlock* ProgressText;

	UFUNCTION(BlueprintCallable, Category = "MiniGame")
	void DrawGameObjects(const FDinoPlayer& Player, const TArray<FDinoObstacle>& Obstacles, bool bGameOver, float SurvivalTime, float VictoryTime);

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