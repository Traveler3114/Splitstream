#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "LobbyPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerReadySignature);

UCLASS()
class ECHOESOFTIME_API ALobbyPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class ULobbyUI> LobbyUIClass;

    UPROPERTY(ReplicatedUsing = OnRep_ReadyState)
    bool bIsReady = false;

    UPROPERTY(ReplicatedUsing = OnRep_TeamTag)
    FGameplayTag TeamTag;

    UFUNCTION()
    void OnRep_ReadyState();

    UFUNCTION(Server, Reliable)
    void ServerSetReadyState(bool bReady);

    UFUNCTION()
    void OnRep_TeamTag();

    UFUNCTION(Server, Reliable)
    void ServerSetTeamTag(FGameplayTag NewTeamTag);


    UPROPERTY(Replicated)
    class ALobbyPlatformActor* AssignedPlatform = nullptr;


    UPROPERTY(BlueprintAssignable, Category = "Events")
    FPlayerReadySignature OnPlayerReady;
};