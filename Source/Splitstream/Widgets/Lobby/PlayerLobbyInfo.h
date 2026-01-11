#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerLobbyInfo.generated.h"

class UButton;
class UTextBlock;
class UImage;
class UTexture2D;
class APlayerState;

UCLASS()
class ECHOESOFTIME_API UPlayerLobbyInfo : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "PlayerLobbyInfo")
    UButton* kick_btn;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "PlayerLobbyInfo")
    UTextBlock* playername_txt;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "PlayerLobbyInfo")
    UImage* avatar_img;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "PlayerLobbyInfo")
    UTextBlock* ready_txt;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "PlayerLobbyInfo")
    UTextBlock* team_txt;

    UFUNCTION(BlueprintCallable, Category = "PlayerLobbyInfo")
    void SetPlayerName(const FText& Name);

    UFUNCTION(BlueprintCallable, Category = "PlayerLobbyInfo")
    void SetAvatarTexture(UTexture2D* Texture);

    UFUNCTION(BlueprintCallable, Category = "PlayerLobbyInfo")
    void SetKickButtonVisible(bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "PlayerLobbyInfo")
    void SetReadyState(bool bReady);

    UFUNCTION(BlueprintCallable, Category = "PlayerLobbyInfo")
    void SetTeamState(const FString& Team);

    UFUNCTION(BlueprintCallable, Category = "PlayerLobbyInfo")
    void SetTargetPlayerState(APlayerState* InTarget);

    UFUNCTION()
    void OnKickButtonClicked();

private:
    UPROPERTY(Transient)
    APlayerState* TargetPlayerState = nullptr;
};