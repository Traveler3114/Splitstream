#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/ICameraDetectable.h"
#include "Interfaces/IGhostMirrorSource.h"

#include "GuardCharacter.generated.h"

class ANavNode;

// Add a simple interface for offset retrieval (optional, or just use AGuardCharacter)
UCLASS()
class ECHOESOFTIME_API AGuardCharacter : public ACharacter, public ICameraDetectable, public IGhostMirrorSource
{
    GENERATED_BODY()

public:
    AGuardCharacter();
    virtual void BeginPlay() override;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost")
    bool bIsInCameraView = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    AActor* TargetActor = nullptr;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ghost", meta = (AllowPrivateAccess = "true"))
    class AGhostCharacterActor* SpawnedGhost = nullptr;

    // GHOST OFFSET IS NOW HERE:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost")
    FVector GhostOffset = FVector(0.0f, 0.0f, -80.0f);

    UFUNCTION(BlueprintCallable, Category = "Ghost")
    FVector GetGhostOffset() const { return GhostOffset; }

    // ICameraDetectable implementation
    virtual void OnDetectedByCamera_Implementation(class ASecurityCamera* Camera) override;
    virtual void OnLostByCamera_Implementation(class ASecurityCamera* Camera) override;

    // IGhostMirrorSource implementation
    virtual bool ShouldGhostBeVisible_Implementation() const override;
    virtual USkeletalMeshComponent* GetMirrorMesh_Implementation() const override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UAISenseConfig_Sight* SightConfig;

    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

};