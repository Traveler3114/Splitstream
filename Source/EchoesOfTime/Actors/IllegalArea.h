// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffect.h"
#include "IllegalArea.generated.h"

class UBoxComponent;
class UGameplayEffect;

/**
 * An area that applies the Illegal.Area tag to characters who enter it.
 * Properly handles overlap events to ensure tags are added/removed correctly.
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
