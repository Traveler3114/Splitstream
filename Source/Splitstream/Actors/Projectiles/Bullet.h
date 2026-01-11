#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bullet.generated.h"

UCLASS()
class SPLITSTREAM_API ABullet : public AActor
{
    GENERATED_BODY()
public:
    ABullet();

    UFUNCTION(BlueprintCallable, Category = "Bullet|Collision")
    void SetIgnoreActorsAndComponents(AActor* IgnoreActor, UPrimitiveComponent* IgnoreComponent);


protected:
    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    TSubclassOf<class UGameplayEffect> DamageEffectClass;

    UPROPERTY(VisibleDefaultsOnly, Category = "Components")
    class USceneComponent* DefaultSceneRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    class UCapsuleComponent* CollisionComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    class UStaticMeshComponent* MeshComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    class UProjectileMovementComponent* ProjectileMovement;

    virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION()
    void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};