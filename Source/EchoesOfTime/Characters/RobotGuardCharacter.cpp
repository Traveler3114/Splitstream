// Fill out your copyright notice in the Description page of Project Settings.


#include "RobotGuardCharacter.h"
#include "Components/StateTreeComponent.h"
#include "AbilitySystem/EOTGameplayTags.h"
#include "Actors/RepairableBase.h"
#include "EngineUtils.h"


void ARobotGuardCharacter::BeginPlay()
{
    Super::BeginPlay();
    for (TActorIterator<ARepairableBase> It(GetWorld()); It; ++It)
    {
        It->OnRequestRepair.AddDynamic(this, &ARobotGuardCharacter::OnRepairRequested);
    }
}

void ARobotGuardCharacter::OnRepairRequested(ARepairableBase* RepairableActor)
{
    if (RepairableActor && !RepairQueue.Contains(RepairableActor))
    {
        RepairQueue.Add(RepairableActor);
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
}

