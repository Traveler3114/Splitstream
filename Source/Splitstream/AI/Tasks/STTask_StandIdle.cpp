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

    // Stop any movement
    UObject* OwnerObject = Context.GetOwner();
    if (AAIController* Controller = Cast<AAIController>(OwnerObject))
    {
        Controller->StopMovement();
    }

    // Get duration from config
    float MinDuration = 2.f;
    float MaxDuration = 6.f;

    AAICharacter* NPC = nullptr;
    if (AAIController* Controller = Cast<AAIController>(OwnerObject))
    {
        NPC = Cast<AAICharacter>(Controller->GetPawn());
    }

    if (NPC)
    {
        if (const FStandIdleBehavior* Params =
            NPC->GetBehaviorParams<FStandIdleBehavior>())
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
        // Idle complete — send decision event to pick next behavior
        Context.SendEvent(TAG_AI_Behavior_Decision);
        return EStateTreeRunStatus::Succeeded;
    }

    return EStateTreeRunStatus::Running;
}
