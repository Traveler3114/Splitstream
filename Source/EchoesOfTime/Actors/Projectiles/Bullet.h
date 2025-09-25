#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bullet.generated.h"

UCLASS()
class ECHOESOFTIME_API ABullet : public AActor
{
    GENERATED_BODY()
public:
    ABullet();

protected:
    UPROPERTY(VisibleDefaultsOnly, Category = "Components")
    class USceneComponent* DefaultSceneRoot;

    UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Components")
    class UCapsuleComponent* CollisionComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    class UStaticMeshComponent* MeshComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    class UProjectileMovementComponent* ProjectileMovement;

    virtual void BeginPlay() override;

    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};