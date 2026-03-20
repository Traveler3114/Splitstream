#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "STTask_NPCBehaviorDecision.generated.h"

// Required by UE5.7 StateTree — even if empty
USTRUCT(BlueprintType)
struct SPLITSTREAM_API FSTTask_NPCBehaviorDecisionInstanceData
{
    GENERATED_BODY()
};

USTRUCT(BlueprintType, meta = (Category = "NPC|Behavior", DisplayName = "NPC Behavior Decision Task"))
struct SPLITSTREAM_API FSTTask_NPCBehaviorDecision : public FStateTreeTaskBase
{
    GENERATED_BODY()

    using FInstanceDataType = FSTTask_NPCBehaviorDecisionInstanceData;

    virtual const UStruct* GetInstanceDataType() const override
    {
        return FInstanceDataType::StaticStruct();
    }

    virtual EStateTreeRunStatus EnterState(
        FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition) const override;

private:
    FGameplayTag SelectRandomWeighted(
        const TArray<FInstancedStruct>& Behaviors) const;

    FGameplayTag SelectSequential(
        const TArray<FInstancedStruct>& Behaviors,
        int32& InOutSequenceIndex) const;
};