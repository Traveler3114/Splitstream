// STTask_Interact.cpp
#include "STTask_Interact.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/Characters/AICharacter.h"
#include "Actors/EnvironmentalObject.h"
#include "Actors/PointActors/EnvironmentalSlot.h"
#include "EngineUtils.h"

EStateTreeRunStatus FSTTask_Interact::EnterState(
    FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);

    AAICharacter* NPC = Cast<AAICharacter>(Context.GetOwner());
    if (!NPC) return EStateTreeRunStatus::Failed;

    AEnvironmentalSlot* Slot = FindBestSlot(NPC);
    if (!Slot) return EStateTreeRunStatus::Failed;

    AAIController* Controller = Cast<AAIController>(NPC->GetController());
    if (!Controller) return EStateTreeRunStatus::Failed;

    // Occupy immediately so no other NPC steals the slot while walking
    Slot->Occupy(NPC);
    Data.OccupiedSlot = Slot;
    Data.bArrived = false;

    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalLocation(Slot->GetActorLocation());
    MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
    Controller->MoveTo(MoveRequest);

    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTTask_Interact::Tick(
    FStateTreeExecutionContext& Context,
    const float DeltaTime) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);

    AAICharacter* NPC = Cast<AAICharacter>(Context.GetOwner());
    if (!NPC) return EStateTreeRunStatus::Failed;

    AAIController* Controller = Cast<AAIController>(NPC->GetController());
    if (!Controller) return EStateTreeRunStatus::Failed;

    if (!Data.bArrived)
    {
        // Waiting to arrive at the slot
        if (Controller->GetMoveStatus() == EPathFollowingStatus::Idle)
        {
            // Snap rotation to match slot facing direction
            NPC->SetActorRotation(Data.OccupiedSlot->GetActorRotation());

            Data.bArrived = true;
            Data.RemainingTime = FMath::FRandRange(MinDuration, MaxDuration);

            // Stop movement cleanly
            Controller->StopMovement();
        }

        return EStateTreeRunStatus::Running;
    }

    // Idling at the slot
    Data.RemainingTime -= DeltaTime;
    if (Data.RemainingTime <= 0.f)
        return EStateTreeRunStatus::Succeeded;

    return EStateTreeRunStatus::Running;
}

void FSTTask_Interact::ExitState(
    FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);

    // Always release the slot regardless of how the state exits
    // (succeeded, failed, or interrupted by another state)
    if (Data.OccupiedSlot)
    {
        Data.OccupiedSlot->Release();
        Data.OccupiedSlot = nullptr;
    }

    AAICharacter* NPC = Cast<AAICharacter>(Context.GetOwner());
    if (NPC)
    {
        if (AAIController* Controller = Cast<AAIController>(NPC->GetController()))
            Controller->StopMovement();
    }

    Data.bArrived = false;
}

AEnvironmentalSlot* FSTTask_Interact::FindBestSlot(AAICharacter* NPC) const
{
    float BestDistSq = FLT_MAX;
    AEnvironmentalSlot* BestSlot = nullptr;

    for (TActorIterator<AEnvironmentalObject> It(NPC->GetWorld()); It; ++It)
    {
        AEnvironmentalObject* Object = *It;
        if (!Object) continue;

        // Must match all required tags
        if (!ObjectTags.IsEmpty() &&
            !Object->ObjectTags.HasAll(ObjectTags)) continue;

        // Must have a free slot
        AEnvironmentalSlot* Slot = Object->GetAvailableSlot();
        if (!Slot) continue;

        // Pick closest object
        const float DistSq = FVector::DistSquared(
            NPC->GetActorLocation(),
            Object->GetActorLocation());

        if (DistSq < BestDistSq)
        {
            BestDistSq = DistSq;
            BestSlot = Slot;
        }
    }

    return BestSlot;
}