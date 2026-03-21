// STTask_BehaviorDecision.cpp
#include "STTask_BehaviorDecision.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "AI/Characters/AICharacter.h"
#include "DataAssets/NPCBehaviorTypes.h"

EStateTreeRunStatus FSTTask_BehaviorDecision::EnterState(
    FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition) const
{
    UObject* Owner = Context.GetOwner();
    AAICharacter* NPC = Cast<AAICharacter>(Owner);
    if (!NPC)
    {
        if (AAIController* Controller = Cast<AAIController>(Owner))
            NPC = Cast<AAICharacter>(Controller->GetPawn());
    }

    if (!NPC || !NPC->BehaviorConfig) return EStateTreeRunStatus::Failed;

    const TArray<FInstancedStruct>& Behaviors = NPC->BehaviorConfig->Behaviors;
    if (Behaviors.IsEmpty()) return EStateTreeRunStatus::Failed;

    const FGameplayTag SelectedTag = SelectWeightedRandom(Behaviors);
    if (!SelectedTag.IsValid()) return EStateTreeRunStatus::Failed;

    Context.SendEvent(SelectedTag);
    return EStateTreeRunStatus::Succeeded;
}

FGameplayTag FSTTask_BehaviorDecision::SelectWeightedRandom(
    const TArray<FInstancedStruct>& Behaviors) const
{
    float TotalWeight = 0.f;
    for (const FInstancedStruct& Entry : Behaviors)
    {
        if (const FNPCBehaviorBase* Base = Entry.GetPtr<FNPCBehaviorBase>())
            TotalWeight += Base->Weight;
    }

    if (TotalWeight <= 0.f) return FGameplayTag::EmptyTag;

    float Roll = FMath::FRandRange(0.f, TotalWeight);
    float Accumulated = 0.f;

    for (const FInstancedStruct& Entry : Behaviors)
    {
        if (const FNPCBehaviorBase* Base = Entry.GetPtr<FNPCBehaviorBase>())
        {
            Accumulated += Base->Weight;
            if (Roll <= Accumulated)
                return Base->BehaviorTag;
        }
    }

    // Fallback to last valid entry
    for (int32 i = Behaviors.Num() - 1; i >= 0; --i)
    {
        if (const FNPCBehaviorBase* Base = Behaviors[i].GetPtr<FNPCBehaviorBase>())
            return Base->BehaviorTag;
    }

    return FGameplayTag::EmptyTag;
}
