#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimelineEra.h"
#include "NavNode.generated.h"


UENUM(BlueprintType)
enum class ENavNodeType : uint8
{
	Ground UMETA(DisplayName = "Ground"),
	Sky    UMETA(DisplayName = "Sky")
};

class USceneComponent;
class UArrowComponent;



UCLASS(BlueprintType, Blueprintable)
class SPLITSTREAM_API ANavNode : public AActor
{
	GENERATED_BODY()

public:
	ANavNode();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	ETimelineEra TimelineEra = ETimelineEra::Past;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NavNode")
	ENavNodeType NodeType = ENavNodeType::Ground;
protected:
	// Shows up fine without Category, but adding one is tidy
	UPROPERTY(VisibleAnywhere, Category = "NavNode")
	TObjectPtr<USceneComponent> Root;
	// Make this editable per-instance in the level Details panel
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "NavNode")
	TObjectPtr<UArrowComponent> ArrowComp; // Add this property

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "NavNode", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<ANavNode>> NeighbourNodes;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "NavNode|Stay")
	bool bStayPoint = false;

public:
	UFUNCTION(BlueprintCallable, Category = "NavNode")
	ANavNode* GetRandomNextNode(const ANavNode* Previous = nullptr) const;

};