#include "UtilityLibrary.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Subsystems/RepairableRegistry.h"
#include "Subsystems/DetectorRegistry.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

float UUtilityLibrary::EstimateTravelTimeBetweenActors(const UObject* WorldContextObject, AActor* StartActor, AActor* EndActor)
{
    if (!WorldContextObject || !StartActor || !EndActor)
        return -1.0f;

    UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
    UNavigationSystemV1* Sys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!Sys)
        return -1.0f;

    UNavigationPath* Path = Sys->FindPathToActorSynchronously(
        World, StartActor->GetActorLocation(), EndActor);

    if (!Path || Path->PathPoints.Num() < 2)
        return -1.0f;

    float PathLength = 0.0f;
    const TArray<FVector>& Points = Path->PathPoints;
    for (int32 i = 1; i < Points.Num(); ++i)
    {
        PathLength += FVector::Dist(Points[i - 1], Points[i]);
    }

    float Speed = 300.f; // Default speed if unknown
    if (const ACharacter* Character = Cast<ACharacter>(StartActor))
    {
        if (const UCharacterMovementComponent* Move = Character->GetCharacterMovement())
            Speed = Move->MaxWalkSpeed;
    }
    if (Speed <= 0.f)
        return -1.f;

    return PathLength / Speed;
}


void UUtilityLibrary::RegisterRepairable(UObject* WorldContext, AActor* Repairable)
{
    if (!WorldContext || !Repairable) return;
    UWorld* World = WorldContext->GetWorld();
    if (!World) return;
    if (URepairableRegistry* Registry = World->GetSubsystem<URepairableRegistry>())
    {
        Registry->RegisterRepairable(Repairable);
    }
}

void UUtilityLibrary::UnregisterRepairable(UObject* WorldContext, AActor* Repairable)
{
    if (!WorldContext || !Repairable) return;
    UWorld* World = WorldContext->GetWorld();
    if (!World) return;
    if (URepairableRegistry* Registry = World->GetSubsystem<URepairableRegistry>())
    {
        Registry->UnregisterRepairable(Repairable);
    }
}

void UUtilityLibrary::RegisterDetector(UObject* WorldContextObject, AActor* Detector)
{
    if (!WorldContextObject || !Detector) return;
    UWorld* World = WorldContextObject->GetWorld();
    if (!World) return;
    if (UDetectorRegistry* Registry = World->GetSubsystem<UDetectorRegistry>())
    {
        Registry->RegisterDetector(Detector);
    }
}

void UUtilityLibrary::UnregisterDetector(UObject* WorldContextObject, AActor* Detector)
{
    if (!WorldContextObject || !Detector) return;
    UWorld* World = WorldContextObject->GetWorld();
    if (!World) return;
    if (UDetectorRegistry* Registry = World->GetSubsystem<UDetectorRegistry>())
    {
        Registry->UnregisterDetector(Detector);
    }
}