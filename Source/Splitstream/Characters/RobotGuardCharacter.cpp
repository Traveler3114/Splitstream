#include "RobotGuardCharacter.h"
#include "Components/StateTreeComponent.h"
#include "AbilitySystem/SplitstreamGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/IRepairable.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"

ARobotGuardCharacter::ARobotGuardCharacter()
{
    CurrentRepairTarget = nullptr;
    PrimaryActorTick.bCanEverTick = false;
}

void ARobotGuardCharacter::BeginPlay()
{
    Super::BeginPlay();
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor->GetClass()->ImplementsInterface(URepairable::StaticClass()))
        {
            if (IRepairable::Execute_GetTimelineEra(Actor) == TimelineEra)
            {
                // Get the delegate (must cast here!):
                IRepairable* Repairable = Cast<IRepairable>(Actor);
                if (Repairable)
                {
                    Repairable->GetOnRepairRequested().AddDynamic(this, &ARobotGuardCharacter::OnRepairRequested);
                }
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
    if (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(TAG_Guard_Status_Repair))
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
            FStateTreeEvent MyEvent(TAG_StateTree_Event_RepairNeeded);
            StateTreeComp->SendStateTreeEvent(MyEvent);
        }
    }
}
void ARobotGuardCharacter::OnRepairFinished()
{
    CurrentRepairTarget = nullptr;
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->RemoveLooseGameplayTag(TAG_Guard_Status_Repair);
    }
    TryStartNextRepair();
}