#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Minigames/FirewallMiniGame.h"
#include "FirewallWidget.generated.h"

UCLASS()
class ECHOESOFTIME_API UFirewallWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* LivesText;
    UPROPERTY(meta = (BindWidget))
    UCanvasPanel* GameCanvas;
    UPROPERTY(meta = (BindWidget))
    UTextBlock* GameOverText;
    UPROPERTY(meta = (BindWidget))
    UTextBlock* BossHPText;

    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void SetLives(int32 NewLives);

    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void SetBossHP(int32 BossHP, int32 MaxHP);

    // Draw everything (including Boss if bHasBoss==true)
    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void DrawGameObjects(
        const FMiniGamePlayer& Player,
        const TArray<FMiniGameEnemy>& Enemies,
        const TArray<FMiniGameHeavyEnemy>& HeavyEnemies,
        const TArray<FMiniGameProjectile>& Projectiles,
        const TArray<FMiniGameEnemyBullet>& EnemyBullets,
        const TArray<FMiniGameHeavyEnemyBullet>& HeavyEnemyBullets,
        bool bHasBoss,
        const FMiniGameBoss& Boss // Only valid if bHasBoss is true
    );

    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void ShowGameOver();
    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void ShowVictory();

private:
    void DrawSprite(FVector2D Position, UTexture2D* Texture, FVector2D Size);
    UImage* GetOrCreateSpriteWidget();
    void HideUnusedSprites();

    int32 InitialChildCount = 0;
    UPROPERTY() TArray<UImage*> SpritePool;
    int32 CurrentSpriteIndex = 0;
};