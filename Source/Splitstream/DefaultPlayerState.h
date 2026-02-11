#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "DefaultPlayerState.generated.h"

class UTexture2D;
class UAbilitySystemComponent;
class UGameplayAbility;
class UAttributeSet;
class UPlayerAttributeSet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerMetaChanged, ADefaultPlayerState*, PS);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadyChanged, ADefaultPlayerState*, PS);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAvatarChanged, ADefaultPlayerState*, PS);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamChanged, ADefaultPlayerState*, PS);

/**
 * Player state for Splitstream. Owns the Ability System Component (ASC)
 * and player attribute set, which persist across pawn respawns and
 * seamless travel (via CopyProperties).
 *
 * Also manages replicated player metadata: display name, avatar,
 * team assignment (FGameplayTag), and lobby ready state.
 */
UCLASS()
class SPLITSTREAM_API ADefaultPlayerState : public APlayerState, public IAbilitySystemInterface
{
    GENERATED_BODY()
public:
    ADefaultPlayerState();
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

    UPlayerAttributeSet* GetAttributeSet() const { return PlayerAttributeSet; }

    virtual void CopyProperties(APlayerState* NewPlayerState) override;

    UFUNCTION(BlueprintPure, Category = "PlayerMeta")
    FString GetDisplayName() const { return DisplayName.IsEmpty() ? GetPlayerName() : DisplayName; }

    UFUNCTION(BlueprintPure, Category = "PlayerMeta")
    int32 GetAvatarIndex() const { return AvatarIndex; }

    UFUNCTION(BlueprintPure, Category = "Lobby")
    bool IsReady() const { return bIsReady; }

    UFUNCTION(BlueprintPure, Category = "Lobby|Team")
    FGameplayTag GetTeamTag() const { return TeamTag; }

    UFUNCTION(BlueprintPure, Category = "Lobby|Team")
    static FString GetTeamDisplayName(const FGameplayTag& Tag);

    UFUNCTION(BlueprintPure, Category = "PlayerMeta")
    UTexture2D* GetAvatarTexture() const { return AvatarTexture; }

    UFUNCTION(BlueprintCallable, Category = "PlayerMeta")
    void SetAvatarTexture(UTexture2D* InTexture);

    // == Team system ==
    UPROPERTY(ReplicatedUsing = OnRep_TeamTag, EditAnywhere, BlueprintReadWrite, Category = "Team")
    FGameplayTag TeamTag;

    UFUNCTION(Server, Reliable)
    void ServerSetTeam(const FGameplayTag& NewTag);

    UFUNCTION(BlueprintCallable, Category = "Lobby|Team")
    void SetTeamLocal(const FGameplayTag& NewTag);

    // == Avatar system ==
    UFUNCTION(BlueprintImplementableEvent, Category = "PlayerMeta")
    void BP_RequestAvatar();

    // == Ready system ==
    UFUNCTION(Server, Reliable) void ServerSetDisplayName(const FString& NewName);
    UFUNCTION(Server, Reliable) void ServerSetAvatarIndex(int32 NewIndex);
    UFUNCTION(Server, Reliable) void ServerSetReady(bool bNewReady);
    UFUNCTION(Server, Reliable) void ServerToggleReady();

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void SetReadyLocal(bool bNewReady);

    UPROPERTY(BlueprintAssignable, Category = "PlayerMeta")
    FOnPlayerMetaChanged OnPlayerMetaChanged;
    UPROPERTY(BlueprintAssignable, Category = "PlayerMeta")
    FOnAvatarChanged OnAvatarChanged;
    UPROPERTY(BlueprintAssignable, Category = "Lobby")
    FOnReadyChanged OnReadyChanged;
    UPROPERTY(BlueprintAssignable, Category = "Lobby")
    FOnTeamChanged OnTeamChanged;

    void UpdateTeamGameplayTag();

protected:
    UPROPERTY(ReplicatedUsing = OnRep_Meta)
    FString DisplayName;
    UPROPERTY(ReplicatedUsing = OnRep_Meta)
    int32 AvatarIndex = 0;
    UPROPERTY(ReplicatedUsing = OnRep_Ready)
    bool bIsReady = false;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "PlayerMeta")
    UTexture2D* AvatarTexture = nullptr;

    UFUNCTION()
    void OnRep_Meta();
    UFUNCTION()
    void OnRep_Ready();
    UFUNCTION()
    void OnRep_TeamTag();

    void ApplyDisplayName(const FString& NewName);
    void ApplyAvatarIndex(int32 NewIndex);
    void ApplyReady(bool bNewReady);

    // TEAM
    void ApplyTeam(const FGameplayTag& NewTag);

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    TObjectPtr<UPlayerAttributeSet> PlayerAttributeSet;
};