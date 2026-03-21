// NPCBehaviorTypes.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InstancedStruct.h"
#include "Engine/DataAsset.h"
#include "AbilitySystem/SplitstreamGameplayTags.h"
#include "NPCBehaviorTypes.generated.h"

// ─────────────────────────────────────────────────────────────
// BASE
// BehaviorTag must match the StateTree state's
// "Required Event to Enter" tag.
// Weight controls how often BehaviorDecision picks this
// behavior relative to others in the same config.
// ─────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct SPLITSTREAM_API FNPCBehaviorBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTag BehaviorTag;

    // Relative weight for random selection by BehaviorDecision task.
    // e.g. Walk=5, Drink=1 means walk is picked ~5x more often.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.1"))
    float Weight = 1.f;

    virtual ~FNPCBehaviorBase() = default;
};

// ─────────────────────────────────────────────────────────────
// WALK AROUND
// ─────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct SPLITSTREAM_API FWalkAroundBehavior : public FNPCBehaviorBase
{
    GENERATED_BODY()

    FWalkAroundBehavior()
    {
        BehaviorTag = TAG_AI_Behavior_WalkAround;
    }

    // Which nav nodes this NPC is allowed to walk to.
    // Leave empty to allow all nodes matching the NPC's TimelineEra.
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTagContainer NavNodeTags;

    // 0 = use character's default MaxWalkSpeed
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
    float MovementSpeed = 0.f;

    // Chance [0-1] to stop and idle when arriving at a StayPoint node
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StopChance = 0.5f;
};

// ─────────────────────────────────────────────────────────────
// STAND IDLE
// ─────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct SPLITSTREAM_API FStandIdleBehavior : public FNPCBehaviorBase
{
    GENERATED_BODY()

    FStandIdleBehavior()
    {
        BehaviorTag = TAG_AI_Behavior_StandIdle;
    }

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
    float MinDuration = 2.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
    float MaxDuration = 6.f;
};

// ─────────────────────────────────────────────────────────────
// CONFIG ASSET
// One asset per NPC variant — e.g. DA_Guard_Patroller
//
// Add one entry per behavior this NPC can perform.
// Not in array = behavior never runs.
// BehaviorDecision task reads this array and does weighted
// random selection, sending the winning tag as a StateTree
// event to activate the matching state.
// ─────────────────────────────────────────────────────────────

UCLASS(BlueprintType)
class SPLITSTREAM_API UNPCBehaviorConfig : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly,
        meta = (BaseStruct = "/Script/Splitstream.NPCBehaviorBase",
                ExcludeBaseStruct))
    TArray<FInstancedStruct> Behaviors;

    // Returns true if this config contains behavior T
    template<typename T>
    bool HasBehavior() const
    {
        return GetBehavior<T>() != nullptr;
    }

    // Returns the params for behavior T, or nullptr if not present
    template<typename T>
    const T* GetBehavior() const
    {
        for (const FInstancedStruct& Entry : Behaviors)
        {
            if (const T* Params = Entry.GetPtr<T>())
                return Params;
        }
        return nullptr;
    }
};
