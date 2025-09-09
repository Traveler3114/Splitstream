#include "NavNode.h"
#include "Components/SceneComponent.h"
#include "Components/ArrowComponent.h"

ANavNode::ANavNode()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
	ArrowComp->SetupAttachment(Root);
}

ANavNode* ANavNode::GetRandomNextNode(const ANavNode* Previous) const
{
	TArray<ANavNode*> Candidates;
	Candidates.Reserve(NeighbourNodes.Num());

	for (ANavNode* Node : NeighbourNodes)
	{
		if (!IsValid(Node) || Node == this) continue;
		if (Previous && Node == Previous) continue;
		Candidates.Add(Node);
	}

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

	if (Candidates.Num() == 0) return nullptr;

	return Candidates[FMath::RandHelper(Candidates.Num())];
}
