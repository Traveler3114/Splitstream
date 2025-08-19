#include "GCN_PastEchoDeactivated.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Actors/TimeObjects/GhostCharacterActor.h" // Include your GhostActor header

bool UGCN_PastEchoDeactivated::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
    if (!MyTarget) return false;

    const APawn* Pawn = Cast<APawn>(MyTarget);
    if (!Pawn || !Pawn->IsLocallyControlled())
    {
        return false;
    }

    UWorld* World = MyTarget->GetWorld();
    if (!World) return false;

    TArray<AActor*> Ghosts;
    UGameplayStatics::GetAllActorsWithTag(World, TEXT("Ghost"), Ghosts);

    int32 SetCount = 0;
    for (AActor* A : Ghosts)
    {
        AGhostCharacterActor* Ghost = Cast<AGhostCharacterActor>(A);
        if (Ghost)
        {
            Ghost->SetIsPastEchoAbilityActive(false);
            ++SetCount;
        }
    }

    return SetCount > 0;
}