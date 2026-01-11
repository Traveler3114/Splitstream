#pragma once

#include "UObject/Interface.h"
#include "IGhostMirrorSource.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UGhostMirrorSource : public UInterface
{
    GENERATED_BODY()
};

class IGhostMirrorSource
{
    GENERATED_BODY()

public:
    /** Should return true if the ghost for this actor should be visible (e.g. in camera view, or other custom logic) */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ghost")
    bool ShouldGhostBeVisible() const;

    /** Must return a valid USkeletalMeshComponent* for pose mirroring */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ghost")
    USkeletalMeshComponent* GetMirrorMesh() const;

    /** Optionally, for more advanced ghosts, can add other info here */
};