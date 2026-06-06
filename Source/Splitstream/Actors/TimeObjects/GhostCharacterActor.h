#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IGhostRevealable.h"
#include "GhostCharacterActor.generated.h"

UCLASS()
class SPLITSTREAM_API AGhostCharacterActor : public AActor, public IGhostRevealable
{
    GENERATED_BODY()

public:
    AGhostCharacterActor();
    virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;

    virtual void SetGhostRevealed_Implementation(bool bRevealed) override;

    void SetIsPastEchoAbilityActive(bool bActive);

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    USceneComponent* DefaultSceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    USkeletalMeshComponent* GhostMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost")
    UMaterialInterface* GhostMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost")
    FVector GhostOffset = FVector(0.0f, 0.0f, -80.0f);

    // Not replicated! Each client can toggle this locally via GameplayCue
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost")
    bool bIsPastEchoAbilityActive = false;

    UFUNCTION(BlueprintCallable, Category = "Ghost")
    void UpdateGhostVisibility();

    // --- Client-side smoothing ---
    UPROPERTY(ReplicatedUsing = OnRep_GhostTargetLocation)
    FVector GhostTargetLocation;
    UPROPERTY(ReplicatedUsing = OnRep_GhostTargetRotation)
    FRotator GhostTargetRotation;

    UFUNCTION()
    void OnRep_GhostTargetLocation();
    UFUNCTION()
    void OnRep_GhostTargetRotation();

    USkeletalMeshComponent* CachedSourceMesh = nullptr;
    bool bMaterialApplied = false;
};