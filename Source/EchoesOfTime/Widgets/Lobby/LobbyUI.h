#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyUI.generated.h"

class UButton;

UCLASS()
class ECHOESOFTIME_API ULobbyUI : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;

    void SetStartButtonEnabled(bool bEnabled);

    void SetStartButtonVisibility(ESlateVisibility InVisibility);

    UPROPERTY(meta = (BindWidget))
    UButton* start_btn;

    UPROPERTY(meta = (BindWidget))
    UButton* leave_btn;

    UPROPERTY(meta = (BindWidget))
    UButton* ready_btn;

    UFUNCTION()
    void OnReadyButtonClicked();

    bool bLocalReady = false;
};