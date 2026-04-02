#include "RobotGuardCharacter.h"
#include "Components/StateTreeComponent.h"
#include "AbilitySystem/SplitstreamGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/IRepairable.h"
#include "AbilitySystemComponent.h"
#include "Subsystems/RepairableRegistry.h"
#include "Engine/World.h"

ARobotGuardCharacter::ARobotGuardCharacter()
{
    CurrentRepairTarget = nullptr;
    PrimaryActorTick.bCanEverTick = false;
}

void ARobotGuardCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (UWorld* World = GetWorld())
    {
        if (URepairableRegistry* Registry = World->GetSubsystem<URepairableRegistry>())
        {
            Registry->OnRepairableRegistered.AddDynamic(this, &ARobotGuardCharacter::HandleRepairableRegistered);
            Registry->OnRepairableUnregistered.AddDynamic(this, &ARobotGuardCharacter::HandleRepairableUnregistered);
        }
    }
}

// Add this handler to bind to new repairables after we've started
void ARobotGuardCharacter::HandleRepairableRegistered(AActor* Actor)
{
    if (Actor && Actor->GetClass()->ImplementsInterface(URepairable::StaticClass()))
    {
        if (IRepairable::Execute_GetTimelineEra(Actor) == TimelineEra)
        {
            IRepairable* Repairable = Cast<IRepairable>(Actor);
            if (Repairable)
            {
                Repairable->GetOnRepairRequested().AddDynamic(this, &ARobotGuardCharacter::OnRepairRequested);
            }
        }
    }
}

void ARobotGuardCharacter::HandleRepairableUnregistered(AActor* Actor)
{
    if (Actor && Actor->GetClass()->ImplementsInterface(URepairable::StaticClass()))
    {
        if (IRepairable::Execute_GetTimelineEra(Actor) == TimelineEra)
        {
            IRepairable* Repairable = Cast<IRepairable>(Actor);
            if (Repairable)
            {
                // ---- CRUCIAL: Remove dynamic binding ----
                Repairable->GetOnRepairRequested().RemoveDynamic(this, &ARobotGuardCharacter::OnRepairRequested);
            }
        }
    }
}

void ARobotGuardCharacter::OnRepairRequested(AActor* Repairable)
{
    QueueRepair(Repairable);
    TryStartNextRepair();
}

void ARobotGuardCharacter::QueueRepair(AActor* Repairable)
{
    if (!Repairable || RepairQueue.Contains(Repairable)) return;

    // TimelineEra check (optional)
    if (Repairable->GetClass()->ImplementsInterface(URepairable::StaticClass()))
    {
        if (IRepairable::Execute_GetTimelineEra(Repairable) != TimelineEra)
            return;
    }
    RepairQueue.Add(Repairable);
}

void ARobotGuardCharacter::TryStartNextRepair()
{
    if (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(TAG_AI_Event_RepairNeeded))
        return;
    if (CurrentRepairTarget)
        return;
    if (RepairQueue.Num() == 0)
        return;
    AActor* NextRepair = RepairQueue[0];
    RepairQueue.RemoveAt(0);
    CurrentRepairTarget = NextRepair;
    AController* GuardController = GetController();
    if (GuardController)
    {
        UStateTreeComponent* StateTreeComp = GuardController->FindComponentByClass<UStateTreeComponent>();
        if (StateTreeComp)
        {
            FStateTreeEvent MyEvent(TAG_AI_Event_RepairNeeded);
            StateTreeComp->SendStateTreeEvent(MyEvent);
        }
    }
}
void ARobotGuardCharacter::OnRepairFinished()
{
    CurrentRepairTarget = nullptr;
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->RemoveLooseGameplayTag(TAG_AI_Event_RepairNeeded);
    }
    TryStartNextRepair();
}