#include "NavUtilityLibrary.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

float UNavUtilityLibrary::EstimateTravelTimeBetweenActors(const UObject* WorldContextObject, AActor* StartActor, AActor* EndActor)
{
    if (!WorldContextObject || !StartActor || !EndActor)
        return -1.0f;

    UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!NavSys)
        return -1.0f;

    UNavigationPath* NavPath = NavSys->FindPathToActorSynchronously(
        World, StartActor->GetActorLocation(), EndActor);

    if (!NavPath || NavPath->PathPoints.Num() < 2)
        return -1.0f;

    float PathLength = 0.0f;
    const TArray<FVector>& Points = NavPath->PathPoints;
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