#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NavNode.generated.h"

UCLASS(BlueprintType, Blueprintable)
class ECHOESOFTIME_API ANavNode : public AActor
{
	GENERATED_BODY()

public:
	ANavNode();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NavNode")
	TObjectPtr<USceneComponent> Root;

	// Connect this node to others (set per-instance in the level)
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "NavNode")
	TArray<TObjectPtr<ANavNode>> NeighbourNodes;

	// Distance threshold to consider the node "reached"
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NavNode", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float ArrivalRadius = 100.f;

	// Draw sphere and arrows in editor/game for quick visualization
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NavNode|Debug")
	bool bDebugDraw = true;

public:
	// Returns a random valid next node. Optionally avoids returning the Previous node to prevent immediate backtracking.
	UFUNCTION(BlueprintCallable, Category = "NavNode")
	ANavNode* GetRandomNextNode(const ANavNode* Previous = nullptr) const;

	// Optional C++-only access to the storage without copying
	const TArray<TObjectPtr<ANavNode>>& GetNeighbourNodesStorage() const { return NeighbourNodes; }

	// Blueprint-friendly accessor that returns a copy
	UFUNCTION(BlueprintPure, Category = "NavNode")
	TArray<ANavNode*> GetNeighbourNodes() const { return TArray<ANavNode*>{NeighbourNodes}; }

	// Expose ArrivalRadius to C++ users safely
	UFUNCTION(BlueprintPure, Category = "NavNode")
	float GetArrivalRadius() const { return ArrivalRadius; }

	// Editor utility: ensure all links are bidirectional (one-click fixup)
	UFUNCTION(CallInEditor, Category = "NavNode|Editor")
	void MakeLinksBidirectional();

protected:
	// Remove invalid/self/duplicate entries
	void SanitizeNeighbours();

	virtual void OnConstruction(const FTransform& Transform) override;
};