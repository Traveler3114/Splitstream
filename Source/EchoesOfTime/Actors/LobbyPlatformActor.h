#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LobbyPlatformActor.generated.h"

class APlayerState;
class UStaticMeshComponent;
class UArrowComponent;
class UWidgetComponent;
class APawn;
class UOpenFriendsListButton;
class UFriendList;
class ADefaultPlayerState;
class UPlayerLobbyInfo;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLobbyPlatformOccupantChanged, ALobbyPlatformActor*, Platform, APlayerState*, NewOccupant);

UCLASS()
class ECHOESOFTIME_API ALobbyPlatformActor : public AActor
{
	GENERATED_BODY()

public:
	ALobbyPlatformActor();

	UFUNCTION(BlueprintPure, Category = "Lobby|Platform")
	bool IsOccupied() const { return OccupantPlayerState != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Lobby|Platform")
	APlayerState* GetOccupant() const { return OccupantPlayerState; }

	UFUNCTION(BlueprintPure, Category = "Lobby|Platform")
	APawn* GetLobbyPawn() const { return OccupantLobbyPawn; }

	UFUNCTION(BlueprintPure, Category = "Lobby|Platform|UI")
	UWidgetComponent* GetOpenFriendListButton() const { return OpenFriendListButton; }

	UFUNCTION(BlueprintPure, Category = "Lobby|Platform|UI")
	UWidgetComponent* GetFriendListWidget() const { return FriendList; }

	UFUNCTION(BlueprintPure, Category = "Lobby|Platform|UI")
	UWidgetComponent* GetPlayerLobbyInfoWidget() const { return PlayerLobbyInfo; }

	UFUNCTION(BlueprintCallable, Category = "Lobby|Platform")
	bool ServerAssignOccupant(APlayerState* NewOccupant);

	UFUNCTION(BlueprintCallable, Category = "Lobby|Platform")
	bool ServerClearOccupant();

	UPROPERTY(BlueprintAssignable, Category = "Lobby|Platform")
	FOnLobbyPlatformOccupantChanged OnOccupantChanged;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Platform")
	USceneComponent* RootScene;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby|Platform")
	UStaticMeshComponent* PlatformMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby|Platform")
	UArrowComponent* SpawnPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby|Platform|UI")
	UWidgetComponent* OpenFriendListButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby|Platform|UI")
	UWidgetComponent* FriendList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby|Platform|UI")
	UWidgetComponent* PlayerLobbyInfo;

	UPROPERTY(ReplicatedUsing = OnRep_OccupantPlayerState)
	APlayerState* OccupantPlayerState = nullptr;

	UFUNCTION()
	void OnRep_OccupantPlayerState();

	void NotifyOccupantChanged();

	void SpawnOccupantPawn();
	void DestroyOccupantPawn();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby|Platform|Display")
	TSubclassOf<APawn> LobbyPawnClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby|Platform|Display")
	bool bDestroyPawnOnClear = true;

	UPROPERTY(Transient)
	APawn* OccupantLobbyPawn = nullptr;

	// Player info widget updates / retry
	void UpdateWidgetsForOccupant();     // also handles PlayerLobbyInfo visibility, kick button visibility, and avatar binding
	void SchedulePlayerInfoRetry();
	void RetryUpdatePlayerInfo();
	UPROPERTY(Transient)
	bool bPendingPlayerInfoRetry = false;

	// Friend list open/close
	void SetFriendListVisible(bool bVisible);
	UFUNCTION()
	void HandleShowFriendListRequested();
	UFUNCTION()
	void HandleShowOpenButtonRequested();

	// Unified rule: button visible only if (unoccupied && friend list hidden)
	void UpdateOpenFriendButtonVisibility();

	// Binding to occupant player state
	UPROPERTY()
	ADefaultPlayerState* CachedDefaultPlayerState = nullptr;
	void BindToOccupantPlayerState();
	void UnbindFromOccupantPlayerState();
	UFUNCTION()
	void HandleOccupantMetaChanged(ADefaultPlayerState* PS);
	UFUNCTION()
	void HandleOccupantReadyChanged(ADefaultPlayerState* PS);
	// New: listen for avatar changes to update the UI
	UFUNCTION()
	void HandleOccupantAvatarChanged(ADefaultPlayerState* PS);
};