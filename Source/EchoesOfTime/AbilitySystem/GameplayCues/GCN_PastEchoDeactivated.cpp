#include "GCN_PastEchoDeactivated.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/IGhostRevealable.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"

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

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    int32 SetCount = 0;
    for (AActor* A : AllActors)
    {
        if (A && A->GetClass()->ImplementsInterface(UGhostRevealable::StaticClass()))
        {
            IGhostRevealable::Execute_SetGhostRevealed(A, false);
            ++SetCount;
        }
    }

    return SetCount > 0;
}