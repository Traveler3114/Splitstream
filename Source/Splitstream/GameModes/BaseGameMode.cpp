#include "BaseGameMode.h"
#include "GameStates/BaseGameState.h"
#include "DefaultGameInstance.h"

void ABaseGameMode::HostLeaveLobby() {
    if (!HasAuthority())
        return;

    FString MenuURL = TEXT("/Game/Maps/MainMenuMap");
    if (ABaseGameState* GS = GetGameState<ABaseGameState>())
    {
        if (!GS->MainMenuMapPath.IsEmpty())
            MenuURL = GS->MainMenuMapPath;
    }
    PendingMenuURL = MenuURL;

    if (UDefaultGameInstance* GI = Cast<UDefaultGameInstance>(GetGameInstance())) {
        GI->HostLeaveToMainMenu(PendingMenuURL);
    }
}