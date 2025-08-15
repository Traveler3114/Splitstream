#include "NavNode.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

ANavNode::ANavNode()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

ANavNode* ANavNode::GetRandomNextNode(const ANavNode* Previous) const
{
	TArray<ANavNode*> Candidates;
	Candidates.Reserve(NeighbourNodes.Num());

	for (ANavNode* Node : NeighbourNodes)
	{
		if (!IsValid(Node) || Node == this)
		{
			continue;
		}

		// Prefer not to immediately go back to where we just came from (if provided)
		if (Previous && Node == Previous)
		{
			continue;
		}

		Candidates.Add(Node);
	}

	// If avoidance filtered all, fall back to any valid next node
	if (Candidates.Num() == 0)
	{
		for (ANavNode* Node : NeighbourNodes)
		{
			if (IsValid(Node) && Node != this)
			{
				Candidates.Add(Node);
			}
		}
	}

	if (Candidates.Num() == 0)
	{
		return nullptr;
	}

	const int32 Index = FMath::RandHelper(Candidates.Num());
	return Candidates[Index];
}

void ANavNode::SanitizeNeighbours()
{
	TSet<ANavNode*> Seen;
	TArray<TObjectPtr<ANavNode>> Cleaned;
	Cleaned.Reserve(NeighbourNodes.Num());

	for (ANavNode* Node : NeighbourNodes)
	{
		if (!IsValid(Node) || Node == this)
		{
			continue;
		}
		if (!Seen.Contains(Node))
		{
			Seen.Add(Node);
			Cleaned.Add(Node);
		}
	}

	if (Cleaned.Num() != NeighbourNodes.Num())
	{
		NeighbourNodes = MoveTemp(Cleaned);
	}
}

void ANavNode::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Keep the list tidy when placed/edited
	SanitizeNeighbours();

	if (!bDebugDraw)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		const FVector Location = GetActorLocation();

		// Editor: persistent, Game: short-lived so it doesn't accumulate
		const bool bIsGameWorld = World->IsGameWorld();
		const bool bPersistentLines = !bIsGameWorld;
		const float LifeTime = bIsGameWorld ? 2.0f : 0.0f;

		// Draw node sphere
		DrawDebugSphere(World, Location, ArrivalRadius, 16, FColor::Green, bPersistentLines, LifeTime, /*DepthPriority*/ 0, /*Thickness*/ 2.f);

		// Draw arrows to next nodes
		for (ANavNode* Node : NeighbourNodes)
		{
			if (!IsValid(Node) || Node == this)
			{
				continue;
			}

			DrawDebugDirectionalArrow(
				World,
				Location,
				Node->GetActorLocation(),
				25.f,
				FColor::Emerald,
				bPersistentLines,
				LifeTime,
				/*DepthPriority*/ 0,
				/*Thickness*/ 2.f
			);
		}
	}
}

void ANavNode::MakeLinksBidirectional()
{
#if WITH_EDITOR
	Modify();
	SanitizeNeighbours();

	for (ANavNode* Node : NeighbourNodes)
	{
		if (!IsValid(Node))
		{
			continue;
		}

		Node->Modify();
		Node->SanitizeNeighbours();

		if (!Node->NeighbourNodes.Contains(this))
		{
			Node->NeighbourNodes.Add(this);
		}
	}
#endif // WITH_EDITOR
}