#include "GCN_PastEchoActivated.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"

bool UGCN_PastEchoActivated::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget) return false;

	const APawn* Pawn = Cast<APawn>(MyTarget);
	if (!Pawn || !Pawn->IsLocallyControlled())
	{
		// Only the locally controlled client's machine should apply the local presentation
		return false;
	}

	UWorld* World = MyTarget->GetWorld();
	if (!World) return false;

	TArray<AActor*> Ghosts;
	UGameplayStatics::GetAllActorsWithTag(World, TEXT("Ghost"), Ghosts);

	for (AActor* A : Ghosts)
	{
		SetActorLocalVisibility(A, /*bVisible*/ true);
	}

	return Ghosts.Num() > 0;
}

void UGCN_PastEchoActivated::SetActorLocalVisibility(AActor* Actor, bool bVisible)
{
	if (!Actor) return;

	const bool bBeforeHidden = Actor->IsHidden();

	int32 SetCount = 0;
	TInlineComponentArray<UPrimitiveComponent*> PrimComps(Actor);
	for (UPrimitiveComponent* C : PrimComps)
	{
		if (!C) continue;
		C->SetVisibility(bVisible, true);
		++SetCount;
	}
}