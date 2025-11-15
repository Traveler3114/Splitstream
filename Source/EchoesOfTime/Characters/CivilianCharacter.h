#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h" // Needed for FOnAttributeChangeData!
#include "AbilitySystemInterface.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"
#include "TimelineEra.h"
#include "CivilianCharacter.generated.h"

UCLASS()
class ECHOESOFTIME_API ACivilianCharacter : public ACharacter, public IAbilitySystemInterface
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
    class ADeskActor* AssignedDesk = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UAbilitySystemComponent* AbilitySystemComponent;

    UPROPERTY()
    class UPlayerAttributeSet* AttributeSet;

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
    TSubclassOf<UGameplayEffect> AttributeInitGE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UAISenseConfig_Sight* SightConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    AActor* TargetActor = nullptr;

    AActor* DetectedActor = nullptr;

    UPROPERTY()
    UTimelineComponent* CivilianTimeline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
    UCurveFloat* CivilianCurve;

    UFUNCTION()
    void OnTimelineFloatUpdate(float Value);

    UFUNCTION()
    void OnTimelineFinished();

protected:
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
};