#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/ICameraDetectable.h"
#include "Interfaces/IGhostMirrorSource.h"
#include "GuardCharacter.generated.h"

class ANavNode;
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

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ghost", meta = (AllowPrivateAccess = "true"))
    class AGhostCharacterActor* SpawnedGhost = nullptr;

    // ICameraDetectable implementation
    virtual void OnDetectedByCamera_Implementation(class ASecurityCamera* Camera) override;
    virtual void OnLostByCamera_Implementation(class ASecurityCamera* Camera) override;

    // IGhostMirrorSource implementation
    virtual bool ShouldGhostBeVisible_Implementation() const override;
    virtual USkeletalMeshComponent* GetMirrorMesh_Implementation() const override;
};