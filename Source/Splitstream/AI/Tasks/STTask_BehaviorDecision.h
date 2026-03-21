// STTask_BehaviorDecision.h
#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "STTask_BehaviorDecision.generated.h"

USTRUCT(BlueprintType)
struct SPLITSTREAM_API FSTTask_BehaviorDecisionInstanceData
{
    GENERATED_BODY()
};

USTRUCT(BlueprintType, meta = (Category = "NPC|Behavior", DisplayName = "Behavior Decision Task"))
struct SPLITSTREAM_API FSTTask_BehaviorDecision : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FSTTask_BehaviorDecisionInstanceData;

    virtual const UStruct* GetInstanceDataType() const override
    {
        return FInstanceDataType::StaticStruct();
    }

    virtual EStateTreeRunStatus EnterState(
        FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition) const override;

private:
    FGameplayTag SelectWeightedRandom(
        const TArray<FInstancedStruct>& Behaviors) const;
};
