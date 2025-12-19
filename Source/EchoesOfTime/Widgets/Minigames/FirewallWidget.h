// Copyright Epic Games, Inc. All Rights Reserved.  

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Minigames/FirewallMiniGame.h"
#include "FirewallWidget.generated.h"

/**
 * Pure UI Widget - Only displays what the game tells it to
 */
UCLASS()
class ECHOESOFTIME_API UFirewallWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    // UI Elements - bind these in Blueprint
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ScoreText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* LivesText;

    UPROPERTY(meta = (BindWidget))
    UCanvasPanel* GameCanvas;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* GameOverText;

    // Called by MiniGame to update UI
    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void SetScore(int32 NewScore);

    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void SetLives(int32 NewLives);

    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void DrawGameObjects(const FMiniGamePlayer& Player, const TArray<FMiniGameEnemy>& Enemies, const TArray<FMiniGameProjectile>& Projectiles);

    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void ShowGameOver();

    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void ShowVictory();

private:
    // Draw functions implemented in C++
    void DrawSprite(FVector2D Position, UTexture2D* Texture, FVector2D Size);
    UImage* GetOrCreateSpriteWidget();
    void HideUnusedSprites();

    // Track initial children count to preserve BP-placed widgets
    int32 InitialChildCount = 0;

    // Object pool for sprite widgets
    UPROPERTY()
    TArray<UImage*> SpritePool;

    int32 CurrentSpriteIndex = 0;
};