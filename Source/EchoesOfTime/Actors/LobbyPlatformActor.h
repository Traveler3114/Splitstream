#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "LobbyPlatformActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKickRequestedPlatform, ALobbyPlatformActor*, Platform);



class UWidgetComponent;

UCLASS()
class ECHOESOFTIME_API ALobbyPlatformActor : public AActor
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintAssignable, Category = "PlayerLobbyInfo")
    FOnKickRequestedPlatform OnKickRequested;

    UFUNCTION()
    void HandleKickRequested();

    ALobbyPlatformActor();
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    // ...

    UFUNCTION()
    void ShowFriendList();
    UFUNCTION()
    void ShowButton();

    UFUNCTION(BlueprintCallable, Category = "Platform")
    APawn* SpawnCharacterAtPlatform(AController* NewController);

    UFUNCTION(BlueprintNativeEvent, Category = "Platform")
    UTexture2D* GetPlayerAvatar(AController* NewController);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
    APawn* OccupyingPawn;

    UFUNCTION(BlueprintCallable, Category = "Platform")
    void SetPlayerReadyState(bool bReady);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
    UWidgetComponent* PlayerInfoWidget;

    UPROPERTY(ReplicatedUsing = OnRep_PlayerInfo)
    FGameplayTag TeamTag;

    UFUNCTION()
    void OnRep_PlayerInfo();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Platform")
    USceneComponent* RootScene;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Platform")
    UStaticMeshComponent* PlatformMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
    UWidgetComponent* OpenFriendsListButtonWidget;

    UPROPERTY(VisibleAnywhere)
    UWidgetComponent* FriendListWidget;



    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
    class UArrowComponent* SpawnPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
    TSubclassOf<APawn> CharacterClassToSpawn;

    UPROPERTY(ReplicatedUsing = OnRep_IsOccupied)
    bool bIsOccupied = false;

    UFUNCTION()
    void OnRep_IsOccupied();

    UPROPERTY(ReplicatedUsing = OnRep_PlayerInfo)
    FString ReplicatedPlayerName;

    UPROPERTY(ReplicatedUsing = OnRep_PlayerInfo)
    UTexture2D* ReplicatedAvatarTexture;



    UPROPERTY(ReplicatedUsing = OnRep_ReadyState)
    bool bIsReady = false;

    UFUNCTION()
    void OnRep_ReadyState();



};