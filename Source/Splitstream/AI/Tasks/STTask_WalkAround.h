#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "STTask_WalkAround.generated.h"

USTRUCT(BlueprintType)
struct SPLITSTREAM_API FSTTask_WalkAroundInstanceData
{
    GENERATED_BODY()

    // Internal — tracks the active EQS request
    UPROPERTY()
    int32 ActiveQueryID = INDEX_NONE;

    // Internal — the node we're currently moving to
    UPROPERTY()
    TObjectPtr<AActor> TargetNode = nullptr;

    // Internal — are we waiting for move to complete
    UPROPERTY()
    bool bIsMoving = false;
};

USTRUCT(BlueprintType, meta = (Category = "NPC|Behavior", DisplayName = "Walk Around Task"))
struct SPLITSTREAM_API FSTTask_WalkAround : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FSTTask_WalkAroundInstanceData;

    // ✅ NOW selectable directly in StateTree (no binding)
    UPROPERTY(EditAnywhere, Category = "EQS")
    TObjectPtr<UEnvQuery> EQSQuery = nullptr;

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
    void RunEQS(FStateTreeExecutionContext& Context) const;
    void OnArrived(FStateTreeExecutionContext& Context) const;
};