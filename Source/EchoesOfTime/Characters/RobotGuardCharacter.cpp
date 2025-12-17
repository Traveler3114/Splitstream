// Fill out your copyright notice in the Description page of Project Settings.


#include "RobotGuardCharacter.h"
#include "Components/StateTreeComponent.h"
#include "AbilitySystem/EOTGameplayTags.h"
#include "Actors/RepairableBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"



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
    if (RepairableActor && !RepairQueue.Contains(RepairableActor) && RepairableActor->TimelineEra == TimelineEra)
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

