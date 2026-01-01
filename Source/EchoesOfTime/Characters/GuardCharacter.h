// GuardCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "AICharacter.h"
#include "Interfaces/IDetectable.h"
#include "Interfaces/IGhostMirrorSource.h"
#include "Interfaces/IInteractable.h"
#include "TimelineEra.h"
#include "AbilitySystemInterface.h"
#include "DataAssets/ItemBase.h"
#include "GameplayEffect.h"
#include "GuardCharacter.generated.h"


class ANavNode;

UCLASS()
class ECHOESOFTIME_API AGuardCharacter : public AAICharacter, public IGhostMirrorSource
{
    GENERATED_BODY()

public:
    AGuardCharacter();
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Guard")
    bool bIsSecurityChief = false;

    UPROPERTY(ReplicatedUsing = OnRep_GuardName, EditAnywhere, BlueprintReadWrite, Category = "Guard")
    FString GuardName;

    UFUNCTION()
    void OnRep_GuardName();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard")
    UTexture2D* PortraitTexture;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guard")
    class UTextRenderComponent* NameText;

    // Assigned Locker
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard")
    class ALockerActor* AssignedLocker = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
    float BaseStayChance = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
    float MinIdleTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
    float MaxIdleTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
    bool bOnlyStayOnMarkedNodes = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nodes")
    ANavNode* CurrentNode = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nodes")
    ANavNode* PreviousNode = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nodes")
    ANavNode* NextNode = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost")
    TSubclassOf<class AGhostCharacterActor> GhostClass;

    UPROPERTY(ReplicatedUsing = OnRep_IsInCameraView, EditAnywhere, BlueprintReadWrite, Category = "Ghost")
    bool bIsInCameraView = false;

    UFUNCTION()
    void OnRep_IsInCameraView();

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ghost", meta = (AllowPrivateAccess = "true"))
    class AGhostCharacterActor* SpawnedGhost = nullptr;

    // IGhostMirrorSource implementation
    virtual bool ShouldGhostBeVisible_Implementation() const override;
    virtual USkeletalMeshComponent* GetMirrorMesh_Implementation() const override;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Alarm")
    float PreAlarmDuration = 3.0f;

    virtual void OnHealthChanged(const struct FOnAttributeChangeData& Data) override;
    virtual void OnDetected_Implementation(AActor* Detector) override;
    virtual void OnLost_Implementation(AActor* Detector) override;
    virtual void OnFullyDetected_Implementation(AActor* ActorDetected) override;
};