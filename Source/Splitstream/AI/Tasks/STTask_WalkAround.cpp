// STTask_WalkAround.cpp
#include "STTask_WalkAround.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "AI/Characters/AICharacter.h"
#include "Actors/PointActors/NavNode.h"
#include "AbilitySystem/SplitstreamGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
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

    AAICharacter* NPC = Cast<AAICharacter>(Context.GetOwner());
    if (!NPC) return EStateTreeRunStatus::Failed;

    AAIController* Controller = Cast<AAIController>(NPC->GetController());
    if (!Controller) return EStateTreeRunStatus::Failed;

    if (Controller->GetMoveStatus() == EPathFollowingStatus::Idle)
    {
        if (OnArrived(Context))
            return EStateTreeRunStatus::Succeeded;
    }

    return EStateTreeRunStatus::Running;
}

void FSTTask_WalkAround::ExitState(
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

    Data.bIsMoving = false;
    Data.TargetNode = nullptr;
}

void FSTTask_WalkAround::MoveToNextNode(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);

    AAICharacter* NPC = Cast<AAICharacter>(Context.GetOwner());
    if (!NPC) return;

    AAIController* Controller = Cast<AAIController>(NPC->GetController());
    if (!Controller) return;

    const bool bFilterByTags = NavNodeTags.Num() > 0;

    TArray<ANavNode*> ValidNodes;
    for (TActorIterator<ANavNode> It(NPC->GetWorld()); It; ++It)
    {
        ANavNode* Node = *It;
        if (!Node) continue;
        if (Node->TimelineEra != NPC->TimelineEra) continue;
        if (bFilterByTags && !Node->NodeTags.HasAny(NavNodeTags)) continue;
        if (Node == Data.TargetNode) continue;
        ValidNodes.Add(Node);
    }

    if (ValidNodes.IsEmpty()) return;

    ANavNode* ChosenNode = ValidNodes[FMath::RandHelper(ValidNodes.Num())];
    Data.TargetNode = ChosenNode;

    if (MovementSpeed > 0.f)
        NPC->GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;

    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalLocation(ChosenNode->GetActorLocation());
    MoveRequest.SetAcceptanceRadius(50.f);
    Controller->MoveTo(MoveRequest);

    Data.bIsMoving = true;
}

bool FSTTask_WalkAround::OnArrived(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);
    Data.bIsMoving = false;

    ANavNode* Node = Cast<ANavNode>(Data.TargetNode);
    if (!Node)
    {
        MoveToNextNode(Context);
        return false;
    }

    if (Node->NodeTags.HasTag(TAG_World_NavNode_StayPoint))
    {
        if (FMath::FRandRange(0.f, 1.f) <= StopChance)
        {
            Context.SendEvent(TAG_AI_Behavior_StandIdle);
            return true;
        }
    }

    MoveToNextNode(Context);
    return false;
}