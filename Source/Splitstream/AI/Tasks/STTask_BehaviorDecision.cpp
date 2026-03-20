#include "STTask_BehaviorDecision.h"
#include "StateTreeExecutionContext.h"
#include "AI/Characters/AICharacter.h"
#include "DataAssets/NPCBehaviorTypes.h"

EStateTreeRunStatus FSTTask_BehaviorDecision::EnterState(
    FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition) const
{
    UObject* OwnerObject = Context.GetOwner();
    if (!OwnerObject)
    {
        UE_LOG(LogTemp, Warning, TEXT("STTask_BehaviorDecision: No valid owner"));
        return EStateTreeRunStatus::Failed;
    }

    AAICharacter* NPC = Cast<AAICharacter>(OwnerObject);
    if (!NPC)
    {
        if (AAIController* Controller = Cast<AAIController>(OwnerObject))
        {
            NPC = Cast<AAICharacter>(Controller->GetPawn());
        }
    }

    if (!NPC || !NPC->BehaviorConfig)
    {
        UE_LOG(LogTemp, Warning, TEXT("STTask_BehaviorDecision: No valid NPC or BehaviorConfig"));
        return EStateTreeRunStatus::Failed;
    }

    const TArray<FInstancedStruct>& Behaviors = NPC->BehaviorConfig->AllowedBehaviors;
    if (Behaviors.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("STTask_BehaviorDecision: AllowedBehaviors is empty"));
        return EStateTreeRunStatus::Failed;
    }

    FGameplayTag SelectedTag;

    switch (NPC->BehaviorConfig->DecisionMode)
    {
    case ENPCDecisionMode::Sequential:
        SelectedTag = SelectSequential(Behaviors, NPC->BehaviorSequenceIndex);
        break;

    case ENPCDecisionMode::RandomWeighted:
    default:
        SelectedTag = SelectRandomWeighted(Behaviors);
        break;
    }

    if (!SelectedTag.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("STTask_BehaviorDecision: Selected behavior tag is invalid"));
        return EStateTreeRunStatus::Failed;
    }

    Context.SendEvent(SelectedTag);

    return EStateTreeRunStatus::Succeeded;
}

FGameplayTag FSTTask_BehaviorDecision::SelectRandomWeighted(
    const TArray<FInstancedStruct>& Behaviors) const
{
    float TotalWeight = 0.f;
    for (const FInstancedStruct& Entry : Behaviors)
    {
        if (const FNPCBehaviorBase* Base = Entry.GetPtr<FNPCBehaviorBase>())
        {
            TotalWeight += Base->Weight;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("STTask_BehaviorDecision: Invalid behavior struct in config"));
        }
    }

    if (TotalWeight <= 0.f)
    {
        for (const FInstancedStruct& Entry : Behaviors)
        {
            if (const FNPCBehaviorBase* Base = Entry.GetPtr<FNPCBehaviorBase>())
                return Base->BehaviorTag;
        }
        return FGameplayTag::EmptyTag;
    }

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

    // Fallback — return last valid behavior
    for (int32 i = Behaviors.Num() - 1; i >= 0; --i)
    {
        if (const FNPCBehaviorBase* Base = Behaviors[i].GetPtr<FNPCBehaviorBase>())
            return Base->BehaviorTag;
    }

    return FGameplayTag::EmptyTag;
}

FGameplayTag FSTTask_BehaviorDecision::SelectSequential(
    const TArray<FInstancedStruct>& Behaviors,
    int32& InOutSequenceIndex) const
{
    if (Behaviors.Num() == 0)
        return FGameplayTag::EmptyTag;

    InOutSequenceIndex = InOutSequenceIndex % Behaviors.Num();

    const FNPCBehaviorBase* Base = Behaviors[InOutSequenceIndex].GetPtr<FNPCBehaviorBase>();
    if (!Base)
    {
        UE_LOG(LogTemp, Warning, TEXT("STTask_BehaviorDecision: Invalid behavior struct at index %d"), InOutSequenceIndex);
        return FGameplayTag::EmptyTag;
    }

    InOutSequenceIndex = (InOutSequenceIndex + 1) % Behaviors.Num();

    return Base->BehaviorTag;
}
