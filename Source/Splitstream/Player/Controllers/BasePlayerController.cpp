#include "BasePlayerController.h"
#include "GameModes/BaseGameMode.h"
#include "GameStates/BaseGameState.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void ABasePlayerController::RequestLeaveToMainMenu()
{
    if (HasAuthority() && IsLocalController())
    {
        if (ABaseGameMode* GM = Cast<ABaseGameMode>(GetWorld()->GetAuthGameMode()))
        {
            // Assumes HostLeaveLobby ultimately flows to ServerTravel
            GM->HostLeaveLobby();
        }
    }
    else
    {
        if (ABaseGameState* GS = GetWorld()->GetGameState<ABaseGameState>())
        {
            //ClientShowLoadingWidget();
            ClientTravel(GS->MainMenuMapPath, TRAVEL_Absolute);
        }
    }
}



//void ABasePlayerController::ClientShowLoadingWidget_Implementation()
//{
//    ShowLoadingWidget();
//}
//
//void ABasePlayerController::ShowLoadingWidget()
//{
//    if (LoadingWidgetClass)
//    {
//        LoadingWidgetInstance = CreateWidget<UUserWidget>(this, LoadingWidgetClass, TEXT("LoadingWidget"));
//        if (LoadingWidgetInstance)
//        {
//            LoadingWidgetInstance->AddToViewport();
//        }
//    }
//}