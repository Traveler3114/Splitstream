// STTask_StandIdle.cpp
#include "STTask_StandIdle.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "AI/Characters/AICharacter.h"
#include "DataAssets/NPCBehaviorTypes.h"
#include "AbilitySystem/SplitstreamGameplayTags.h"

EStateTreeRunStatus FSTTask_StandIdle::EnterState(
    FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);

    UObject* Owner = Context.GetOwner();

    // Stop any movement
    if (AAIController* Controller = Cast<AAIController>(Owner))
        Controller->StopMovement();

    // Get duration from config
    float MinDuration = 2.f;
    float MaxDuration = 6.f;

    AAICharacter* NPC = nullptr;
    if (AAIController* Controller = Cast<AAIController>(Owner))
        NPC = Cast<AAICharacter>(Controller->GetPawn());

    if (NPC && NPC->BehaviorConfig)
    {
        if (const FStandIdleBehavior* Params = NPC->BehaviorConfig->GetBehavior<FStandIdleBehavior>())
        {
            MinDuration = Params->MinDuration;
            MaxDuration = Params->MaxDuration;
        }
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
    {
        // Send Decision event so BehaviorDecision picks the next behavior
        Context.SendEvent(TAG_AI_Behavior_Decision);
        return EStateTreeRunStatus::Succeeded;
    }

    return EStateTreeRunStatus::Running;
}
