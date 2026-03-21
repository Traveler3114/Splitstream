// STTask_WalkAround.cpp
#include "STTask_WalkAround.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "AI/Characters/AICharacter.h"
#include "Actors/PointActors/NavNode.h"
#include "DataAssets/NPCBehaviorTypes.h"
#include "AbilitySystem/SplitstreamGameplayTags.h"
#include "EngineUtils.h"

EStateTreeRunStatus FSTTask_WalkAround::EnterState(
    FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition) const
{
    MoveToNextNode(Context);
    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTTask_WalkAround::Tick(
    FStateTreeExecutionContext& Context,
    const float DeltaTime) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);

    if (!Data.bIsMoving) return EStateTreeRunStatus::Running;

    AAIController* Controller = Cast<AAIController>(Context.GetOwner());
    if (!Controller) return EStateTreeRunStatus::Failed;

    if (Controller->GetMoveStatus() == EPathFollowingStatus::Idle)
        OnArrived(Context);

    return EStateTreeRunStatus::Running;
}

void FSTTask_WalkAround::ExitState(
    FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);

    if (AAIController* Controller = Cast<AAIController>(Context.GetOwner()))
        Controller->StopMovement();

    Data.bIsMoving = false;
    Data.TargetNode = nullptr;
}

void FSTTask_WalkAround::MoveToNextNode(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);

    AAIController* Controller = Cast<AAIController>(Context.GetOwner());
    if (!Controller) return;

    AAICharacter* NPC = Cast<AAICharacter>(Controller->GetPawn());
    if (!NPC) return;

    // Get allowed nav node tags from config
    FGameplayTagContainer AllowedTags;
    bool bFilterByTags = false;

    if (NPC->BehaviorConfig)
    {
        if (const FWalkAroundBehavior* Params = NPC->BehaviorConfig->GetBehavior<FWalkAroundBehavior>())
        {
            AllowedTags = Params->NavNodeTags;
            bFilterByTags = AllowedTags.Num() > 0;
        }
    }

    // Gather valid nodes
    TArray<ANavNode*> ValidNodes;
    for (TActorIterator<ANavNode> It(NPC->GetWorld()); It; ++It)
    {
        ANavNode* Node = *It;
        if (!Node) continue;
        if (Node->TimelineEra != NPC->TimelineEra) continue;
        if (bFilterByTags && !Node->NodeTags.HasAny(AllowedTags)) continue;
        if (Node == Data.TargetNode) continue;
        ValidNodes.Add(Node);
    }

    if (ValidNodes.IsEmpty()) return;

    ANavNode* ChosenNode = ValidNodes[FMath::RandHelper(ValidNodes.Num())];
    Data.TargetNode = ChosenNode;

    // Apply movement speed override if set
    if (NPC->BehaviorConfig)
    {
        if (const FWalkAroundBehavior* Params = NPC->BehaviorConfig->GetBehavior<FWalkAroundBehavior>())
        {
            if (Params->MovementSpeed > 0.f)
                NPC->GetCharacterMovement()->MaxWalkSpeed = Params->MovementSpeed;
        }
    }

    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalLocation(ChosenNode->GetActorLocation());
    MoveRequest.SetAcceptanceRadius(50.f);
    Controller->MoveTo(MoveRequest);

    Data.bIsMoving = true;
}

void FSTTask_WalkAround::OnArrived(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);
    Data.bIsMoving = false;

    ANavNode* Node = Cast<ANavNode>(Data.TargetNode);
    if (!Node)
    {
        MoveToNextNode(Context);
        return;
    }

    // Check if this is a StayPoint and roll for idle
    if (Node->NodeTags.HasTag(TAG_World_NavNode_StayPoint))
    {
        float StopChance = 0.5f;

        AAIController* Controller = Cast<AAIController>(Context.GetOwner());
        AAICharacter* NPC = Controller ? Cast<AAICharacter>(Controller->GetPawn()) : nullptr;

        if (NPC && NPC->BehaviorConfig)
        {
            if (const FWalkAroundBehavior* Params = NPC->BehaviorConfig->GetBehavior<FWalkAroundBehavior>())
                StopChance = Params->StopChance;
        }

        if (FMath::FRandRange(0.f, 1.f) <= StopChance)
        {
            // Hand off to Idle state, which will send Decision when done
            Context.SendEvent(TAG_AI_Behavior_StandIdle);
            return;
        }
    }

    MoveToNextNode(Context);
}
