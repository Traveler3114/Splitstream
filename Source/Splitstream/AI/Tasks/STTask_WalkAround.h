// STTask_WalkAround.h
#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
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
    void OnArrived(FStateTreeExecutionContext& Context) const;
};
