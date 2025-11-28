#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "Interfaces/IDetectable.h"
#include "AbilitySystemInterface.h"
#include "TimelineEra.h"
#include "CivilianCharacter.generated.h"

class ADeskActor;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UPlayerAttributeSet;

UCLASS()
class ECHOESOFTIME_API ACivilianCharacter : public ACharacter, public IAbilitySystemInterface, public IDetectable
{
    GENERATED_BODY()

public:
    ACivilianCharacter();
    virtual void BeginPlay() override;

    void OnHealthChanged(const struct FOnAttributeChangeData& Data);

    

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian")
    FString CivilianName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian")
    UTexture2D* PortraitTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian")
    ADeskActor* AssignedDesk = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UAbilitySystemComponent* AbilitySystemComponent;

    UPROPERTY()
    UPlayerAttributeSet* AttributeSet;

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
    TSubclassOf<UGameplayEffect> AttributeInitGE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    AActor* TargetActor = nullptr;

    AActor* DetectedActor = nullptr;


    virtual void OnFullyDetected_Implementation(AActor* DetectingActor) override;

    virtual bool IsActorAlreadyDetected_Implementation(AActor* DetectingActor) const override;

protected:
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
};