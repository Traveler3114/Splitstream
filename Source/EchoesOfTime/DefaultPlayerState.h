#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "DefaultPlayerState.generated.h"

class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerMetaChanged, ADefaultPlayerState*, PS);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadyChanged, ADefaultPlayerState*, PS);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAvatarChanged, ADefaultPlayerState*, PS);

UCLASS()
class ECHOESOFTIME_API ADefaultPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	ADefaultPlayerState();

	UFUNCTION(BlueprintPure, Category = "PlayerMeta")
	FString GetDisplayName() const { return DisplayName.IsEmpty() ? GetPlayerName() : DisplayName; }

	UFUNCTION(BlueprintPure, Category = "PlayerMeta")
	int32 GetAvatarIndex() const { return AvatarIndex; }

	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool IsReady() const { return bIsReady; }

	UFUNCTION(BlueprintPure, Category = "PlayerMeta")
	UTexture2D* GetAvatarTexture() const { return AvatarTexture; }

	UFUNCTION(BlueprintCallable, Category = "PlayerMeta")
	void SetAvatarTexture(UTexture2D* InTexture);

	// Blueprint event you call to fetch avatar
	UFUNCTION(BlueprintImplementableEvent, Category = "PlayerMeta")
	void BP_RequestAvatar();

	// Server RPCs (called by owning client in networked play)
	UFUNCTION(Server, Reliable) void ServerSetDisplayName(const FString& NewName);
	UFUNCTION(Server, Reliable) void ServerSetAvatarIndex(int32 NewIndex);
	UFUNCTION(Server, Reliable) void ServerSetReady(bool bNewReady);
	UFUNCTION(Server, Reliable) void ServerToggleReady();

	// Local path for Standalone/Authority so UI can change readiness without RPC
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetReadyLocal(bool bNewReady);

	UPROPERTY(BlueprintAssignable, Category = "PlayerMeta") FOnPlayerMetaChanged OnPlayerMetaChanged;
	UPROPERTY(BlueprintAssignable, Category = "PlayerMeta") FOnAvatarChanged     OnAvatarChanged;
	UPROPERTY(BlueprintAssignable, Category = "Lobby")      FOnReadyChanged     OnReadyChanged;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Meta) FString DisplayName;
	UPROPERTY(ReplicatedUsing = OnRep_Meta) int32   AvatarIndex = 0;
	UPROPERTY(ReplicatedUsing = OnRep_Ready) bool   bIsReady = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "PlayerMeta")
	UTexture2D* AvatarTexture = nullptr;

	UFUNCTION() void OnRep_Meta();
	UFUNCTION() void OnRep_Ready();

	void ApplyDisplayName(const FString& NewName);
	void ApplyAvatarIndex(int32 NewIndex);
	void ApplyReady(bool bNewReady);

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};