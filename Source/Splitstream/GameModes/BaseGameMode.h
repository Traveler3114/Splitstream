#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BaseGameMode.generated.h"

UCLASS()
class SPLITSTREAM_API ABaseGameMode : public AGameMode
{
    GENERATED_BODY()
public:
    // Used by Leave logic to store intended URL 
    FString PendingMenuURL;

    /** Leaves to Main Menu */
    UFUNCTION(BlueprintCallable, Category = "Base|Leave")
    virtual void HostLeaveLobby();

protected:
    /** Override to add custom leave logic in child class if needed */
};