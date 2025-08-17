#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "DefaultPlayerState.generated.h"

class UTexture2D;
class UAbilitySystemComponent;
class UGameplayAbility;
class UAttributeSet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerMetaChanged, ADefaultPlayerState*, PS);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadyChanged, ADefaultPlayerState*, PS);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAvatarChanged, ADefaultPlayerState*, PS);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamChanged, ADefaultPlayerState*, PS);

UCLASS()
class ECHOESOFTIME_API ADefaultPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	ADefaultPlayerState();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	void GiveAbilities();

	UFUNCTION(BlueprintPure, Category = "PlayerMeta")
	FString GetDisplayName() const { return DisplayName.IsEmpty() ? GetPlayerName() : DisplayName; }

	UFUNCTION(BlueprintPure, Category = "PlayerMeta")
	int32 GetAvatarIndex() const { return AvatarIndex; }

	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool IsReady() const { return bIsReady; }

	UFUNCTION(BlueprintPure, Category = "Lobby|Team")
	FString GetTeamName() const { return TeamName; }

	UFUNCTION(BlueprintPure, Category = "PlayerMeta")
	UTexture2D* GetAvatarTexture() const { return AvatarTexture; }

	UFUNCTION(BlueprintCallable, Category = "PlayerMeta")
	void SetAvatarTexture(UTexture2D* InTexture);

	// == Team system ==
	UPROPERTY(ReplicatedUsing = OnRep_TeamName) FString TeamName = "Past"; // Past/Future

	UFUNCTION(Server, Reliable) void ServerSetTeam(const FString& NewTeam);
	UFUNCTION(BlueprintCallable, Category = "Lobby|Team") void SetTeamLocal(const FString& NewTeam);

	// == Avatar system ==
	UFUNCTION(BlueprintImplementableEvent, Category = "PlayerMeta")
	void BP_RequestAvatar();

	// == Ready system ==
	UFUNCTION(Server, Reliable) void ServerSetDisplayName(const FString& NewName);
	UFUNCTION(Server, Reliable) void ServerSetAvatarIndex(int32 NewIndex);
	UFUNCTION(Server, Reliable) void ServerSetReady(bool bNewReady);
	UFUNCTION(Server, Reliable) void ServerToggleReady();

	UFUNCTION(BlueprintCallable, Category = "Lobby") void SetReadyLocal(bool bNewReady);

	UPROPERTY(BlueprintAssignable, Category = "PlayerMeta") FOnPlayerMetaChanged OnPlayerMetaChanged;
	UPROPERTY(BlueprintAssignable, Category = "PlayerMeta") FOnAvatarChanged     OnAvatarChanged;
	UPROPERTY(BlueprintAssignable, Category = "Lobby")      FOnReadyChanged     OnReadyChanged;
	UPROPERTY(BlueprintAssignable, Category = "Lobby")      FOnTeamChanged      OnTeamChanged;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Meta) FString DisplayName;
	UPROPERTY(ReplicatedUsing = OnRep_Meta) int32   AvatarIndex = 0;
	UPROPERTY(ReplicatedUsing = OnRep_Ready) bool   bIsReady = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "PlayerMeta")
	UTexture2D* AvatarTexture = nullptr;

	UFUNCTION() void OnRep_Meta();
	UFUNCTION() void OnRep_Ready();
	UFUNCTION() void OnRep_TeamName();

	void ApplyDisplayName(const FString& NewName);
	void ApplyAvatarIndex(int32 NewIndex);
	void ApplyReady(bool bNewReady);

	// TEAM
	void ApplyTeam(const FString& NewTeam);
	void UpdateTeamGameplayTag();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
};