#include "RobotGuardCharacter.h"
#include "Components/StateTreeComponent.h"
#include "AbilitySystem/EOTGameplayTags.h"
#include "Actors/RepairableBase.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"

ARobotGuardCharacter::ARobotGuardCharacter()
{
    // Set default value if needed
    CurrentRepairTarget = nullptr;
}

void ARobotGuardCharacter::BeginPlay()
{
    Super::BeginPlay();

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARepairableBase::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        ARepairableBase* Repairable = Cast<ARepairableBase>(Actor);
        if (Repairable && Repairable->TimelineEra == TimelineEra)
        {
            Repairable->OnRequestRepair.AddDynamic(this, &ARobotGuardCharacter::OnRepairRequested);
        }
    }
}

void ARobotGuardCharacter::OnRepairRequested(ARepairableBase* RepairableActor)
{
    QueueRepair(RepairableActor);
    TryStartNextRepair();
}

void ARobotGuardCharacter::QueueRepair(ARepairableBase* RepairableActor)
{
    if (!RepairableActor || RepairQueue.Contains(RepairableActor) || RepairableActor->TimelineEra != TimelineEra)
        return;
    RepairQueue.Add(RepairableActor);
}

void ARobotGuardCharacter::TryStartNextRepair()
{
    if (AbilitySystemComponent->HasMatchingGameplayTag(TAG_Guard_Status_Repair))
        return; // Already repairing

    if (CurrentRepairTarget)
        return; // Defensive, for double protection

    if (RepairQueue.Num() == 0)
        return; // Nothing to do

    // Get next
    ARepairableBase* NextRepair = RepairQueue[0];
    RepairQueue.RemoveAt(0);
    CurrentRepairTarget = NextRepair;

    // Send StateTree event with NextRepair as "payload"
    // If you use a variable (easier): set a BP-accessible property "CurrentRepairTarget" and read that in your StateTree BP task.

    AController* GuardController = GetController();
    if (GuardController)
    {
        UStateTreeComponent* StateTreeComp = GuardController->FindComponentByClass<UStateTreeComponent>();
        if (StateTreeComp)
        {
            // Optionally, if your StateTree event can take a payload in C++:
            // FStateTreeEvent MyEvent(TAG_StateTree_Event_RepairNeeded, NextRepair); // if you support payloads
            // StateTreeComp->SendStateTreeEvent(MyEvent);

            // Or just use the "CurrentRepairTarget" property—set above—for Blueprint to read in StateTree Task
            FStateTreeEvent MyEvent(TAG_StateTree_Event_RepairNeeded);
            StateTreeComp->SendStateTreeEvent(MyEvent);
        }
    }
}

void ARobotGuardCharacter::OnRepairFinished()
{
    // Call this from your StateTree repair task/BP when the repair is actually completed

    CurrentRepairTarget = nullptr;

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->RemoveLooseGameplayTag(TAG_Guard_Status_Repair);
    }

    // See if there's another repair waiting
    TryStartNextRepair();
}