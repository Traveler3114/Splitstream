#include "STTask_WalkAround.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "AI/Characters/AICharacter.h"
#include "Actors/PointActors/NavNode.h"
#include "DataAssets/NPCBehaviorTypes.h"
#include "AbilitySystem/SplitstreamGameplayTags.h"

EStateTreeRunStatus FSTTask_WalkAround::EnterState(
    FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition) const
{
    RunEQS(Context);
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
    {
        OnArrived(Context);
    }

    return EStateTreeRunStatus::Running;
}

void FSTTask_WalkAround::ExitState(
    FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);

    if (Data.ActiveQueryID != INDEX_NONE)
    {
        if (UEnvQueryManager* EQSManager =
            UEnvQueryManager::GetCurrent(Context.GetOwner()->GetWorld()))
        {
            EQSManager->AbortQuery(Data.ActiveQueryID);
        }
        Data.ActiveQueryID = INDEX_NONE;
    }

    if (AAIController* Controller = Cast<AAIController>(Context.GetOwner()))
    {
        Controller->StopMovement();
    }

    Data.bIsMoving = false;
    Data.TargetNode = nullptr;
}

void FSTTask_WalkAround::RunEQS(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);

    // ✅ FIX: use task property instead of instance data
    if (!EQSQuery)
    {
        UE_LOG(LogTemp, Warning, TEXT("STTask_WalkAround: No EQS query assigned"));
        return;
    }

    AAIController* Controller = Cast<AAIController>(Context.GetOwner());
    if (!Controller) return;

    AAICharacter* NPC = Cast<AAICharacter>(Controller->GetPawn());
    if (!NPC) return;

    UEnvQueryManager* EQSManager = UEnvQueryManager::GetCurrent(NPC->GetWorld());
    if (!EQSManager) return;

    // ✅ FIX: use EQSQuery from task
    FEnvQueryRequest QueryRequest(EQSQuery, NPC);

    Data.ActiveQueryID = QueryRequest.Execute(
        EEnvQueryRunMode::SingleResult,
        FQueryFinishedSignature::CreateLambda(
            [this, &Context](TSharedPtr<FEnvQueryResult> Result)
            {
                FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
                InstanceData.ActiveQueryID = INDEX_NONE;

                if (!Result.IsValid() || Result->IsAborted())
                    return;

                AActor* ResultActor = Result->GetItemAsActor(0);
                if (!ResultActor) return;

                InstanceData.TargetNode = ResultActor;

                AAIController* AIController = Cast<AAIController>(Context.GetOwner());
                if (!AIController) return;

                AAICharacter* NPC = Cast<AAICharacter>(AIController->GetPawn());
                if (!NPC) return;

                if (const FWalkAroundBehavior* Params =
                    NPC->GetBehaviorParams<FWalkAroundBehavior>())
                {
                    if (Params->MovementSpeed > 0.f)
                    {
                        NPC->GetCharacterMovement()->MaxWalkSpeed =
                            Params->MovementSpeed;
                    }
                }

                FAIMoveRequest MoveRequest;
                MoveRequest.SetGoalLocation(ResultActor->GetActorLocation());
                MoveRequest.SetAcceptanceRadius(50.f);

                AIController->MoveTo(MoveRequest);

                InstanceData.bIsMoving = true;
            }));
}

void FSTTask_WalkAround::OnArrived(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);
    Data.bIsMoving = false;

    ANavNode* Node = Cast<ANavNode>(Data.TargetNode);
    if (!Node)
    {
        RunEQS(Context);
        return;
    }

    if (Node->NodeTags.HasTag(TAG_World_NavNode_StayPoint))
    {
        AAIController* Controller = Cast<AAIController>(Context.GetOwner());
        AAICharacter* NPC = Controller
            ? Cast<AAICharacter>(Controller->GetPawn())
            : nullptr;

        float StopChance = 0.5f;

        if (NPC)
        {
            if (const FWalkAroundBehavior* Params =
                NPC->GetBehaviorParams<FWalkAroundBehavior>())
            {
                StopChance = Params->StopChance;
            }
        }

        if (FMath::FRandRange(0.f, 1.f) <= StopChance)
        {
            Context.SendEvent(TAG_AI_Behavior_StandIdle);
            return;
        }
    }

    RunEQS(Context);
}