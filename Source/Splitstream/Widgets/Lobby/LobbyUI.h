#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "LobbyUI.generated.h"

class UButton;
class UTextBlock;
class ADefaultPlayerState;

UCLASS()
class SPLITSTREAM_API ULobbyUI : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;

    void SetStartButtonEnabled(bool bEnabled);
    void SetStartButtonVisibility(ESlateVisibility InVisibility);

    UFUNCTION()
    void OnChangeTeamButtonClicked();
    UFUNCTION()
    void OnStartButtonClicked();
    UFUNCTION()
    void OnLeaveButtonClicked();
    UFUNCTION()
    void OnReadyButtonClicked();

    UPROPERTY(meta = (BindWidget)) UButton* start_btn;
    UPROPERTY(meta = (BindWidget)) UButton* leave_btn;
    UPROPERTY(meta = (BindWidget)) UButton* ready_btn;
    UPROPERTY(meta = (BindWidget)) UButton* changeteam_btn;

    UPROPERTY(meta = (BindWidget)) UTextBlock* team_txt;
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly) UTextBlock* ready_txt;
    UPROPERTY(meta = (BindWidget)) UTextBlock* MapName_txt;

    UFUNCTION(BlueprintCallable, Category = "LobbyUI")
    void SetMapName(const FText& Name);

private:
    void RefreshReadyLabel(bool bIsReady);
    void RefreshTeamLabel(const FGameplayTag& TeamTag);
    UFUNCTION()
    void HandleLocalReadyChanged(ADefaultPlayerState* PS);
    UFUNCTION()
    void HandleLocalTeamChanged(ADefaultPlayerState* PS);
};