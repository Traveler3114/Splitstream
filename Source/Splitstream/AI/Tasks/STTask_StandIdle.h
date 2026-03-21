#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "STTask_StandIdle.generated.h"

USTRUCT(BlueprintType)
struct SPLITSTREAM_API FSTTask_StandIdleInstanceData
{
    GENERATED_BODY()

    // Countdown timer — set on enter, counts down in Tick
    UPROPERTY()
    float RemainingTime = 0.f;
};

USTRUCT(BlueprintType, meta = (Category = "NPC|Behavior", DisplayName = "Stand Idle Task"))
struct SPLITSTREAM_API FSTTask_StandIdle : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FSTTask_StandIdleInstanceData;

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
};
