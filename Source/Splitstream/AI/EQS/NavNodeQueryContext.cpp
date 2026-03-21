#include "NavNodeQueryContext.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "AI/Characters/AICharacter.h"
#include "DataAssets/NPCBehaviorTypes.h"
#include "InstancedStruct.h"
#include "Actors/PointActors/NavNode.h"

void UNavNodeQueryContext::ProvideContext(FEnvQueryInstance& QueryInstance,
    FEnvQueryContextData& ContextData) const
{
    AAICharacter* NPC = Cast<AAICharacter>(QueryInstance.Owner.Get());
    if (!NPC) return;

    FGameplayTagContainer AllowedTags;
    bool bFilterByTags = false;

    if (NPC->BehaviorConfig)
    {
        for (const FInstancedStruct& Entry : NPC->BehaviorConfig->AllowedBehaviors)
        {
            if (const FWalkAroundBehavior* WalkParams = Entry.GetPtr<FWalkAroundBehavior>())
            {
                AllowedTags = WalkParams->NavNodeTags;
                bFilterByTags = AllowedTags.Num() > 0;
                break;
            }
        }
    }

    TArray<AActor*> MatchingNodes;
    for (TActorIterator<ANavNode> It(NPC->GetWorld()); It; ++It)
    {
        ANavNode* Node = *It;
        if (!Node) continue;
        if (Node->TimelineEra != NPC->TimelineEra) continue;
        if (bFilterByTags && !Node->NodeTags.HasAny(AllowedTags)) continue;

        MatchingNodes.Add(Node);
    }

    UEnvQueryItemType_Actor::SetContextHelper(ContextData, MatchingNodes);
}
