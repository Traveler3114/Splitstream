// STTask_Interact.cpp
#include "STTask_Interact.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/Characters/AICharacter.h"
#include "AI/Characters/CivilianCharacter.h"
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

    AEnvironmentalObject* TargetObject = nullptr;
    AEnvironmentalSlot* Slot = FindBestSlot(NPC, TargetObject);
    if (!Slot) return EStateTreeRunStatus::Failed;

    AAIController* Controller = Cast<AAIController>(NPC->GetController());
    if (!Controller) return EStateTreeRunStatus::Failed;

    // Occupy immediately so no other NPC steals the slot while walking
    Slot->Occupy(NPC);
    Data.OccupiedSlot = Slot;
    Data.TargetObject = TargetObject;
    Data.bArrived = false;
    Data.bMovementStarted = false;

    // Track current destination on the civilian so FindBestSlot can
    // exclude it on subsequent iterations.
    if (ACivilianCharacter* Civilian = Cast<ACivilianCharacter>(NPC))
        Civilian->CurrentSearchable = TargetObject;

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
        const EPathFollowingStatus::Type MoveStatus = Controller->GetMoveStatus();

        // Wait for at least one tick where movement is confirmed to have started
        // before treating Idle as "arrived", to avoid the race where GetMoveStatus
        // still returns Idle on the very first frame after MoveTo is called.
        if (!Data.bMovementStarted)
        {
            if (MoveStatus != EPathFollowingStatus::Idle)
                Data.bMovementStarted = true;

            return EStateTreeRunStatus::Running;
        }

        // Waiting to arrive at the slot
        if (MoveStatus == EPathFollowingStatus::Idle)
        {
            // Snap rotation to match slot facing direction
            NPC->SetActorRotation(Data.OccupiedSlot->GetActorRotation());

            Data.bArrived = true;
            Data.RemainingTime = FMath::FRandRange(MinDuration, MaxDuration);

            // Stop movement cleanly
            Controller->StopMovement();

            // Let the object know an NPC has interacted with it.
            // For civilians interacting with objects that have a SearchComponent,
            // this leaves fingerprints and makes the object searchable by the player.
            if (Data.TargetObject)
                Data.TargetObject->Interact_Implementation(NPC);
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
        // Record the object we just visited so FindBestSlot avoids it next time.
        if (ACivilianCharacter* Civilian = Cast<ACivilianCharacter>(NPC))
        {
            Civilian->LastSearchable = Data.TargetObject;
            Civilian->CurrentSearchable = nullptr;
        }

        if (AAIController* Controller = Cast<AAIController>(NPC->GetController()))
            Controller->StopMovement();
    }

    Data.TargetObject = nullptr;
    Data.bArrived = false;
    Data.bMovementStarted = false;
}

AEnvironmentalSlot* FSTTask_Interact::FindBestSlot(AAICharacter* NPC, AEnvironmentalObject*& OutObject) const
{
    // For civilians, avoid revisiting the object they just came from.
    AEnvironmentalObject* LastVisited = nullptr;
    if (const ACivilianCharacter* Civilian = Cast<ACivilianCharacter>(NPC))
        LastVisited = Civilian->LastSearchable;

    // Search helper: iterates all matching objects and picks the nearest one.
    // When SkipObject is non-null, that specific object is excluded.
    auto SearchObjects = [&](AEnvironmentalObject* SkipObject,
        AEnvironmentalSlot*& OutSlot, AEnvironmentalObject*& OutObj)
    {
        float BestDistSq = FLT_MAX;
        OutSlot = nullptr;
        OutObj = nullptr;

        for (TActorIterator<AEnvironmentalObject> It(NPC->GetWorld()); It; ++It)
        {
            AEnvironmentalObject* Object = *It;
            if (!Object || Object == SkipObject) continue;

            if (!ObjectTags.IsEmpty() &&
                !Object->ObjectTags.HasAll(ObjectTags)) continue;

            AEnvironmentalSlot* Slot = Object->GetAvailableSlot();
            if (!Slot) continue;

            const float DistSq = FVector::DistSquared(
                NPC->GetActorLocation(),
                Object->GetActorLocation());

            if (DistSq < BestDistSq)
            {
                BestDistSq = DistSq;
                OutSlot = Slot;
                OutObj = Object;
            }
        }
    };

    AEnvironmentalSlot* BestSlot = nullptr;
    SearchObjects(LastVisited, BestSlot, OutObject);

    // If skipping LastVisited left us with nothing, fall back to including it.
    if (!BestSlot && LastVisited)
        SearchObjects(nullptr, BestSlot, OutObject);

    return BestSlot;
}