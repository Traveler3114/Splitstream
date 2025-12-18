// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffect.h"
#include "IllegalArea.generated.h"

class UBoxComponent;
class UGameplayEffect;

/**
 * An area that applies the Character.Status.Illegal.Area tag to characters who enter it.
 * Properly handles overlap events to ensure tags are added/removed correctly.
 * 
 * This class fixes the issue where illegal tags could get stuck on players due to:
 * - Multiple overlap events triggering duplicate effect applications
 * - Effects not being properly removed on overlap end
 * - Network replication timing issues
 * 
 * The class maintains a map of active effect handles per actor, ensuring:
 * - Only one effect is applied per actor at a time
 * - Effects are always properly removed when the actor leaves
 * - Server-authoritative effect application for multiplayer safety
 * 
 * Usage:
 * 1. Create an instance of AIllegalArea in the level
 * 2. Set the IllegalAreaEffect property to GE_Illegal (or your illegal area effect)
 *    - The effect should grant the Character.Status.Illegal.Area tag
 *    - Recommended: Use an infinite duration effect that's removed manually
 * 3. Adjust the AreaVolume extent to define the illegal zone size
 * 4. The tag will automatically be applied/removed as characters enter/exit
 */
UCLASS()
class ECHOESOFTIME_API AIllegalArea : public AActor
{
	GENERATED_BODY()
	
public:	
	AIllegalArea();

protected:
	virtual void BeginPlay() override;

	/** The collision component that defines the illegal area */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Illegal Area")
	UBoxComponent* AreaVolume;

	/** The gameplay effect class to apply when a character enters the area */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Illegal Area")
	TSubclassOf<UGameplayEffect> IllegalAreaEffect;

	/** Map of actors to their active gameplay effect handles */
	UPROPERTY()
	TMap<AActor*, FActiveGameplayEffectHandle> ActiveEffectHandles;

	/** Called when something enters the illegal area */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, 
		const FHitResult& SweepResult);

	/** Called when something leaves the illegal area */
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
