#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/DefaultAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/DefaultAttributeSet.h"
#include "DefaultPlayerState.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerReadySignature);

UCLASS()
class ECHOESOFTIME_API ADefaultPlayerState : public APlayerState, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    ADefaultPlayerState();

    // IAbilitySystemInterface
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    TObjectPtr<UDefaultAbilitySystemComponent> AbilitySystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    TObjectPtr<UDefaultAttributeSet> AttributeSet;



    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;



    UPROPERTY(BlueprintAssignable, Category = "Events")
    FPlayerReadySignature OnPlayerReady;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing= OnRep_AssignedPlatform)
    class ALobbyPlatformActor* AssignedPlatform = nullptr;

    UFUNCTION()
    void OnRep_AssignedPlatform();


    UPROPERTY(ReplicatedUsing = OnRep_ReadyState)
    bool bIsReady = false;

    UFUNCTION()
    void OnRep_ReadyState();

    UFUNCTION(Server, Reliable)
    void ServerSetReadyState(bool bReady);


    UPROPERTY(ReplicatedUsing = OnRep_TeamTag)
    FGameplayTag TeamTag;

    UFUNCTION()
    void OnRep_TeamTag();

    UFUNCTION(Server, Reliable)
    void ServerSetTeamTag(FGameplayTag NewTeamTag);

};