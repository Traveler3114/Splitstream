#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "DefaultPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerMetaChanged, ADefaultPlayerState*, PS); // name/avatar/role changed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadyChanged, ADefaultPlayerState*, PS);

UCLASS()
class ECHOESOFTIME_API ADefaultPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	ADefaultPlayerState();

	// Display name (fallback to APlayerState::GetPlayerName if empty)
	UFUNCTION(BlueprintPure, Category = "PlayerMeta")
	FString GetDisplayName() const { return DisplayName.IsEmpty() ? GetPlayerName() : DisplayName; }

	UFUNCTION(BlueprintPure, Category = "PlayerMeta")
	int32 GetAvatarIndex() const { return AvatarIndex; }

	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool IsReady() const { return bIsReady; }

	// Server RPCs (called by owning client)
	UFUNCTION(Server, Reliable) void ServerSetDisplayName(const FString& NewName);
	UFUNCTION(Server, Reliable) void ServerSetAvatarIndex(int32 NewIndex);
	UFUNCTION(Server, Reliable) void ServerSetReady(bool bNewReady);
	UFUNCTION(Server, Reliable) void ServerToggleReady();

	UPROPERTY(BlueprintAssignable) FOnPlayerMetaChanged OnPlayerMetaChanged;
	UPROPERTY(BlueprintAssignable) FOnReadyChanged OnReadyChanged;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Meta) FString DisplayName;
	UPROPERTY(ReplicatedUsing = OnRep_Meta) int32   AvatarIndex = 0;
	UPROPERTY(ReplicatedUsing = OnRep_Ready) bool  bIsReady = false;

	UFUNCTION() void OnRep_Meta();
	UFUNCTION() void OnRep_Ready();

	void ApplyDisplayName(const FString& NewName);
	void ApplyAvatarIndex(int32 NewIndex);
	void ApplyReady(bool bNewReady);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};