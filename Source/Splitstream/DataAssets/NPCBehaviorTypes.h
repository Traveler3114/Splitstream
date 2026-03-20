// NPCBehaviorTypes.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InstancedStruct.h"
#include "Engine/DataAsset.h"
#include "AbilitySystem/SplitstreamGameplayTags.h"
#include "NPCBehaviorTypes.generated.h"

// ─────────────────────────────────────────────────────────────
// DECISION MODE
// ─────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENPCDecisionMode : uint8
{
    RandomWeighted  UMETA(DisplayName = "Random Weighted"),
    Sequential      UMETA(DisplayName = "Sequential"),
    Hybrid          UMETA(DisplayName = "Hybrid")
};

// ─────────────────────────────────────────────────────────────
// BASE
// ─────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct SPLITSTREAM_API FNPCBehaviorBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTag BehaviorTag;

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTagContainer NavNodeTags;

    // 0 = use character default
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
    float MovementSpeed = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StopChance = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bOnlyStopOnMarkedNodes = true;
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
// One asset per NPC variant e.g. DA_Guard_Patroller
// ─────────────────────────────────────────────────────────────

UCLASS(BlueprintType)
class SPLITSTREAM_API UNPCBehaviorConfig : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    ENPCDecisionMode DecisionMode = ENPCDecisionMode::RandomWeighted;

    UPROPERTY(EditAnywhere, BlueprintReadOnly,
        meta = (BaseStruct = "/Script/Splitstream.NPCBehaviorBase",
                ExcludeBaseStruct))
    TArray<FInstancedStruct> AllowedBehaviors;
};