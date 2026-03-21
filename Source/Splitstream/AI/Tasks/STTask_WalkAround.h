// STTask_WalkAround.h
#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "STTask_WalkAround.generated.h"

USTRUCT(BlueprintType)
struct SPLITSTREAM_API FSTTask_WalkAroundInstanceData
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<AActor> TargetNode = nullptr;

    UPROPERTY()
    bool bIsMoving = false;
};

USTRUCT(BlueprintType, meta = (Category = "NPC|Behavior", DisplayName = "Walk Around Task"))
struct SPLITSTREAM_API FSTTask_WalkAround : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FSTTask_WalkAroundInstanceData;

    virtual const UStruct* GetInstanceDataType() const override
    {
        return FInstanceDataType::StaticStruct();
    }

    // Bind these to StateTree Parameters in the editor

    // Which nav nodes this NPC is allowed to walk to.
    // Leave empty to allow all nodes matching the NPC's TimelineEra.
    UPROPERTY(EditAnywhere)
    FGameplayTagContainer NavNodeTags;

    // 0 = use character's default MaxWalkSpeed
    UPROPERTY(EditAnywhere)
    float MovementSpeed = 0.f;

    // Chance [0-1] to stop and idle when arriving at a StayPoint node
    UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StopChance = 0.5f;

    virtual EStateTreeRunStatus EnterState(
        FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition) const override;

    virtual EStateTreeRunStatus Tick(
        FStateTreeExecutionContext& Context,
        const float DeltaTime) const override;

    virtual void ExitState(
        FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition) const override;

private:
    void MoveToNextNode(FStateTreeExecutionContext& Context) const;

    // Returns true if idle roll won and walk should stop
    bool OnArrived(FStateTreeExecutionContext& Context) const;
};