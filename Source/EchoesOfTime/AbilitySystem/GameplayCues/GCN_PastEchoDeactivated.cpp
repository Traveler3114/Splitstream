#include "GCN_PastEchoDeactivated.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"


bool UGCN_PastEchoDeactivated::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{

	UWorld* World = MyTarget->GetWorld();

	TArray<AActor*> Ghosts;
	UGameplayStatics::GetAllActorsWithTag(World, TEXT("Ghost"), Ghosts);


	for (AActor* A : Ghosts)
	{
		SetActorLocalVisibility(A, /*bVisible*/ false);
	}

	return Ghosts.Num() > 0;
}

void UGCN_PastEchoDeactivated::SetActorLocalVisibility(AActor* Actor, bool bVisible)
{
	if (!Actor) return;

	const bool bBeforeHidden = Actor->IsHidden();
	Actor->SetActorHiddenInGame(!bVisible);

	int32 SetCount = 0;
	TInlineComponentArray<UPrimitiveComponent*> PrimComps(Actor);
	for (UPrimitiveComponent* C : PrimComps)
	{
		if (!C) continue;
		C->SetVisibility(bVisible, true);
		++SetCount;
	}
}