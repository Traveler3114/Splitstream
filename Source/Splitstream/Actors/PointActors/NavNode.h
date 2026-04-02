#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "TimelineEra.h"
#include "NavNode.generated.h"

class USceneComponent;
class UArrowComponent;

UCLASS(BlueprintType, Blueprintable)
class SPLITSTREAM_API ANavNode : public AActor, public IGameplayTagAssetInterface
{
    GENERATED_BODY()

public:
    ANavNode();

    virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override
    {
        TagContainer.AppendTags(NodeTags);
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NavNode")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NavNode")
    FGameplayTagContainer NodeTags;

protected:
    UPROPERTY(VisibleAnywhere, Category = "NavNode")
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NavNode")
    TObjectPtr<UArrowComponent> ArrowComp;
};