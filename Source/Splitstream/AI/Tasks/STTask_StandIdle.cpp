// STTask_StandIdle.cpp
#include "STTask_StandIdle.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "AI/Characters/AICharacter.h"

EStateTreeRunStatus FSTTask_StandIdle::EnterState(
    FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);

    AAICharacter* NPC = Cast<AAICharacter>(Context.GetOwner());
    if (NPC)
    {
        if (AAIController* NPCController = Cast<AAIController>(NPC->GetController()))
            NPCController->StopMovement();
    }

    Data.RemainingTime = FMath::FRandRange(MinDuration, MaxDuration);
    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTTask_StandIdle::Tick(
    FStateTreeExecutionContext& Context,
    const float DeltaTime) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);

    Data.RemainingTime -= DeltaTime;

    if (Data.RemainingTime <= 0.f)
        return EStateTreeRunStatus::Succeeded;

    return EStateTreeRunStatus::Running;
}