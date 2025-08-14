#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "DefaultPlayerState.generated.h"

class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerMetaChanged, ADefaultPlayerState*, PS); // name/avatar/role changed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadyChanged, ADefaultPlayerState*, PS);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAvatarChanged, ADefaultPlayerState*, PS);

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

	// Avatar texture stored per-client (not replicated)
	UFUNCTION(BlueprintPure, Category = "PlayerMeta")
	UTexture2D* GetAvatarTexture() const { return AvatarTexture; }

	// Called by your Blueprint (after you fetched Steam avatar) to store it in PlayerState
	UFUNCTION(BlueprintCallable, Category = "PlayerMeta")
	void SetAvatarTexture(UTexture2D* InTexture);


	// Fired when AvatarTexture changes (UI can update)
	UPROPERTY(BlueprintAssignable, Category = "PlayerMeta")
	FOnAvatarChanged OnAvatarChanged;

	// You implement this in BP. C++ calls it on clients to request you to fetch avatar using UniqueId.
	UFUNCTION(BlueprintImplementableEvent, Category = "PlayerMeta")
	void BP_RequestAvatar();

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

	// Client-only (transient) avatar texture; not replicated
	UPROPERTY(Transient, BlueprintReadOnly, Category = "PlayerMeta")
	UTexture2D* AvatarTexture = nullptr;

	UFUNCTION() void OnRep_Meta();
	UFUNCTION() void OnRep_Ready();

	void ApplyDisplayName(const FString& NewName);
	void ApplyAvatarIndex(int32 NewIndex);
	void ApplyReady(bool bNewReady);

	// Ask Blueprint to fetch avatar shortly after BeginPlay on clients
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};