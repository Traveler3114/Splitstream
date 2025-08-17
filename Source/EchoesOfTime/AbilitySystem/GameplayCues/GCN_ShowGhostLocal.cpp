#include "GCN_ShowGhostLocal.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"

bool UGCN_ShowGhostLocal::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget) return false;

	// Only the owning client's machine should apply the local presentation.
	const APawn* Pawn = Cast<APawn>(MyTarget);
	if (!Pawn || !Pawn->IsLocallyControlled())
	{
		return false;
	}

	UWorld* World = MyTarget->GetWorld();
	if (!World) return false;

	// Find all "Ghost" tagged actors and reveal them locally.
	TArray<AActor*> Ghosts;
	UGameplayStatics::GetAllActorsWithTag(World, TEXT("Ghost"), Ghosts);

	for (AActor* A : Ghosts)
	{
		SetActorLocalVisibility(A, /*bVisible*/ true);
	}

	// TODO: Add local-only SFX/VFX here later (e.g., UGameplayStatics::PlaySound2D, Niagara, etc.)

	return Ghosts.Num() > 0;
}

void UGCN_ShowGhostLocal::SetActorLocalVisibility(AActor* Actor, bool bVisible)
{
	if (!Actor) return;

	// Local-only: does not replicate.
	Actor->SetActorHiddenInGame(!bVisible);

	TInlineComponentArray<UPrimitiveComponent*> PrimComps(Actor);
	for (UPrimitiveComponent* C : PrimComps)
	{
		if (!C) continue;
		C->SetVisibility(bVisible, true);
		// Optionally gate collisions locally:
		// C->SetCollisionEnabled(bVisible ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}
}