#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "GameplayEffect.h"
#include "AbilitySystemInterface.h"
#include "IllegalArea.generated.h"

class UBoxComponent;

UCLASS()
class ECHOESOFTIME_API AIllegalArea : public AActor
{
    GENERATED_BODY()
    
public:    
    AIllegalArea();

protected:
    virtual void BeginPlay() override;

    // Component for area trigger
    UPROPERTY(VisibleAnywhere)
    UBoxComponent* TriggerBox;

    // The Gameplay Effect class to apply (set this in BP or C++)
    UPROPERTY(EditAnywhere, Category="GAS")
    TSubclassOf<UGameplayEffect> GameplayEffectClass;

    // Map to store handles per actor (now supports multiple handles per actor)
    TMap<TWeakObjectPtr<AActor>, TArray<FActiveGameplayEffectHandle>> EffectHandles;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};